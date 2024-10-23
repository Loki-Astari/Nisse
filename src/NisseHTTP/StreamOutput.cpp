#include "StreamOutput.h"

using namespace ThorsAnvil::Nisse::HTTP;

StreamBufOutput::~StreamBufOutput()
{
    done();
}

StreamBufOutput::StreamBufOutput(Complete&& complete)
    : remaining{0}
    , buffer{nullptr}
    , chunked{false}
    , firstChunk{false}
    , complete{std::move(complete)}
    , chunkBuffer{0}
{}

StreamBufOutput::StreamBufOutput(std::ostream& stream, std::streamsize length, Complete&& complete)
    : remaining{length}
    , buffer{stream.rdbuf()}
    , chunked{false}
    , firstChunk{false}
    , complete{std::move(complete)}
    , chunkBuffer{0}
{}

StreamBufOutput::StreamBufOutput(std::ostream& stream, Encoding /*encoding*/, Complete&& complete)
    : remaining{chunkBufferSize}
    , buffer{stream.rdbuf()}
    , chunked{true}
    , firstChunk{true}
    , complete{std::move(complete)}
    , chunkBuffer{0}
{
    chunkBuffer.resize(chunkBufferSize);
}

StreamBufOutput::StreamBufOutput(StreamBufOutput&& move) noexcept
    : remaining{std::exchange(move.remaining, 0)}
    , buffer{std::exchange(move.buffer, nullptr)}
    , chunked{std::exchange(move.chunked, false)}
    , firstChunk{std::exchange(move.firstChunk, false)}
    , complete{std::exchange(move.complete, [](){})}
    , chunkBuffer{std::move(move.chunkBuffer)}
{}

StreamBufOutput& StreamBufOutput::operator=(StreamBufOutput&& move) noexcept
{
    remaining   = 0;
    buffer      = nullptr;
    chunked     = false;
    firstChunk  = false;
    complete    = [](){};
    chunkBuffer.resize(0);

    swap(move);

    return *this;
}

void StreamBufOutput::swap(StreamBufOutput& other) noexcept
{
    std::streambuf::swap(other);

    using std::swap;
    swap(remaining,     other.remaining);
    swap(buffer,        other.buffer);
    swap(chunked,       other.chunked);
    swap(firstChunk,    other.firstChunk);
    swap(complete,      other.complete);
    swap(chunkBuffer,   other.chunkBuffer);
}

// Control:
int StreamBufOutput::sync()
{
    //std::cerr << "sync\n";
    checkBuffer();
    dumpBuffer();
    return buffer->pubsync();
}

void StreamBufOutput::dumpBuffer()
{
    //std::cerr << "dumpBuffer\n";
    if (chunked)
    {
        std::streamsize     chunkSize = chunkBufferSize - remaining;
        if (chunkSize != 0)
        {
            outputChunkSize(chunkSize);
            sendAllData(&chunkBuffer[0], chunkBufferSize - remaining);
            sendAllData("\r\n", 2);
            remaining = chunkBufferSize;
        }
    }
}

void StreamBufOutput::done()
{
    //std::cerr << "Done\n";
    if (chunked)
    {
        dumpBuffer();
        //std::cerr << "Sending Tail\n";
        sendAllData("0\r\n\r\n", 5);
        remaining = 0;
        chunked = false;
        //std::cerr << "Sync Buffer\n";
        buffer->pubsync();
    }
}

void StreamBufOutput::sendAllData(char const* s, std::streamsize size)
{
    //std::cerr << "sendAllData\n";
    while (size != 0)
    {
        std::streamsize sent = buffer->sputn(s, size);
        s    += sent;
        size -= sent;
    }
}

char StreamBufOutput::toHex(int digit)
{
    return digit >= 10 ? 'A' + (digit - 10)
                       : '0' + digit;
}

void StreamBufOutput::outputChunkSize(std::streamsize size)
{
    bool started   = false;
    for (auto x: { 4096, 256, 16, 1})
    {
        int digit = size / x;
        if (digit != 0 || started)
        {
            started = true;
            buffer->sputc(toHex(digit));
        }
        size = size - (digit * x);
    }
    sendAllData("\r\n", 2);
}

// Write:
std::streamsize StreamBufOutput::xsputnChunked(char_type const* s,std::streamsize count)
{
    //std::cerr << "xsputnChunked\n";
    if (count > remaining)
    {
        std::streamsize     chunkSize = chunkBufferSize - remaining + count;
        outputChunkSize(chunkSize);
        sendAllData(&chunkBuffer[0], chunkBufferSize - remaining);
        sendAllData(s, count);
        sendAllData("\r\n", 2);
        remaining = chunkBufferSize;
    }
    else
    {
        std::copy(s, s + count, std::begin(chunkBuffer) + ( chunkBufferSize - remaining));
        remaining -= count;
    }
    return count;
}

std::streamsize StreamBufOutput::xsputnLength(char_type const* s,std::streamsize count)
{
    //std::cerr << "xsputnLength\n";
    std::streamsize result = 0;
    while (remaining != 0 && count != 0)
    {
        std::size_t max = std::min(count, remaining);
        std::streamsize sent = buffer->sputn(s, max);
        s           += sent;
        count       -= sent;
        result      += sent;
        remaining   -= sent;
    }
    if (remaining == 0)
    {
        sync();
    }
    return result;
}
std::streamsize StreamBufOutput::xsputn(char_type const* s,std::streamsize count)
{
    //std::cerr << "xsputn\n";
    if (chunked) {
        return xsputnChunked(s, count);
    }
    else {
        return xsputnLength(s, count);
    }
}

StreamBufOutput::int_type StreamBufOutput::overflowChunked(int_type ch)
{
    //std::cerr << "overflowChunked\n";
    if (ch != traits::eof())
    {
        char_type v = ch;
        xsputnChunked(&v, 1);
    }
    return 1;
}

StreamBufOutput::int_type StreamBufOutput::overflowLength(int_type ch)
{
    //std::cerr << "overflowLength\n";
    if (ch == traits::eof()) {
        return remaining == 0 ? traits::eof() : 1;
    }
    int_type result = traits::eof();
    if (remaining != 0)
    {
        if ((result = buffer->sputc(ch)) != traits::eof()) {
            --remaining;
        }
        if (remaining == 0) {
            sync();
        }
    }
    return result;
}

StreamBufOutput::int_type StreamBufOutput::overflow(int_type ch)
{
    //std::cerr << "overflow\n";
    if (chunked) {
        return overflowChunked(ch);
    }
    else {
        return overflowLength(ch);
    }
}

void StreamBufOutput::checkBuffer()
{
}
