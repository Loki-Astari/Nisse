#include "StreamInput.h"

using namespace ThorsAnvil::Nisse::HTTP;

StreamBufInput::StreamBufInput(Complete&& complete)
    : remaining{0}
    , processed{0}
    , buffer{nullptr}
    , chunked{false}
    , firstChunk{false}
    , complete{std::move(complete)}
    , chunkBuffer{}
{}

StreamBufInput::StreamBufInput(std::istream& stream, BodyEncoding encoding, Complete&& complete)
    : remaining{0}
    , processed{0}
    , buffer{stream.rdbuf()}
    , chunked{false}
    , firstChunk{false}
    , complete{std::move(complete)}
    , chunkBuffer{}
{
    struct BodyEncodingInit
    {
        StreamBufInput* self;
        BodyEncodingInit(StreamBufInput* self)
            : self(self)
        {}
        void operator()(std::size_t contentLength)      {self->remaining = contentLength;}
        void operator()(std::streamsize contentLength)  {self->remaining = contentLength;}
        void operator()(Encoding /*encoding*/)          {self->chunked = true; self->firstChunk = true;}
    };
    std::visit(BodyEncodingInit{this}, encoding);
    chunkBuffer.resize(chunkBufferSize);
}

StreamBufInput::StreamBufInput(StreamBufInput&& move) noexcept
    : remaining{std::exchange(move.remaining, 0)}
    , processed{std::exchange(move.processed, 0)}
    , buffer{std::exchange(move.buffer, nullptr)}
    , chunked{std::exchange(move.chunked, false)}
    , firstChunk{std::exchange(move.firstChunk, false)}
    , complete{std::exchange(move.complete, [](){})}
    , chunkBuffer{std::move(move.chunkBuffer)}
{}

StreamBufInput& StreamBufInput::operator=(StreamBufInput&& move) noexcept
{
    remaining   = 0;
    processed   = 0;
    buffer      = nullptr;
    chunked     = false;
    firstChunk  = false;
    complete    = [](){};
    chunkBuffer.resize(0);

    swap(move);

    return *this;
}

void StreamBufInput::swap(StreamBufInput& other) noexcept
{
    std::streambuf::swap(other);

    using std::swap;
    swap(remaining,     other.remaining);
    swap(processed,     other.processed);
    swap(buffer,        other.buffer);
    swap(chunked,       other.chunked);
    swap(firstChunk,    other.firstChunk);
    swap(complete,      other.complete);
    swap(chunkBuffer,   other.chunkBuffer);
}

StreamBufInput::int_type StreamBufInput::underflow()
{
    if (remaining == 0)
    {
        getNextChunk();
        if (remaining == 0) {
            return traits::eof();
        }
    }

    std::streamsize get = std::min(remaining, chunkBufferSize);
    std::streamsize got = buffer->sgetn(&chunkBuffer[0], get);
    if (got == 0) {
        return traits::eof();
    }
    remaining -= got;
    processed += (egptr() - eback());
    setg(&chunkBuffer[0], &chunkBuffer[0], &chunkBuffer[0] + got);
    return chunkBuffer[0];
}

std::streamsize StreamBufInput::xsgetn(char_type* s, std::streamsize count)
{
    std::streamsize got   = 0;
    std::streamsize avail = egptr() - gptr();
    std::streamsize get   = std::min(count, avail);

    std::copy(gptr(), gptr() + get, s);
    if (get == avail)
    {
        processed += (egptr() - eback());
        setg(&chunkBuffer[0], &chunkBuffer[0], &chunkBuffer[0]);
    }
    else {
        gbump(get);
    }

    got += get;
    s += get;
    count -= get;
    while (count != 0)
    {
        if (remaining == 0)
        {
            getNextChunk();
            if (remaining == 0) {
                break;
            }
        }

        get = std::min(count, remaining);
        std::streamsize extra = buffer->sgetn(s, get);
        processed += extra;
        if (extra == 0) {
            break;
        }
        got += extra;
        s += extra;
        count -= extra;
        remaining -= extra;
    }
    return got;
}

StreamBufInput::pos_type StreamBufInput::seekpos(StreamBufInput::pos_type pos, std::ios_base::openmode which)
{
    pos_type                current = static_cast<pos_type>(currentPosition());
    off_type                off     = pos - current;

    return seekoff(off, std::ios_base::cur, which);
}

StreamBufInput::pos_type StreamBufInput::seekoff(StreamBufInput::off_type off, std::ios_base::seekdir way, std::ios_base::openmode which)
{
    if (which != std::ios_base::in) {
        return 0;
    }
    if (way != std::ios_base::cur) {
        return currentPosition();
    }
    if (off < 0) {
        return currentPosition();
    }
    if (off == 0) {
        return currentPosition();
    }
    std::streamsize count = off;
    std::streamsize avail = egptr() - gptr();
    if (count < avail)
    {
        gbump(count);
        return currentPosition();
    }
    gbump(avail);
    processed += (egptr() - eback());
    setg(&chunkBuffer[0], &chunkBuffer[0], &chunkBuffer[0]);
    count -= avail;

    while (count != 0)
    {
        if (remaining == 0)
        {
            getNextChunk();
            if (remaining == 0) {
                break;
            }
        }

        std::streamsize get = std::min(count, remaining);
        std::streamsize extra = buffer->pubseekoff(get, std::ios_base::cur, std::ios_base::in);
        if (extra == -1) {
            break;
        }
        processed += get;
        remaining -= get;
        count -= get;
    }
    return currentPosition();
}

void StreamBufInput::checkBuffer()
{
}

void StreamBufInput::getNextChunk()
{
    if (!chunked)
    {
        complete();
        return;
    }
    if (firstChunk) {
        firstChunk = false;
    }
    else
    {
        // Each chunk terminated by '\r\n'
        buffer->sbumpc();
        buffer->sbumpc();
    }

    int next;
    while ((next = buffer->sbumpc()) != traits::eof())
    {
        if (next == '\r')
        {
            next = buffer->sbumpc();    // Check it is '\n'
            break;
        }
        next = (next >= 'a' && next <= 'f') ? next - 'a' + 10
             : (next >= 'A' && next <= 'F') ? next - 'A' + 10
             : (next >= '0' && next <= '9') ? next - '0'
             : 0;

        remaining = (remaining * 16) + next;
    }
    if (remaining == 0)
    {
        chunked = false;
        complete();
    }
}
