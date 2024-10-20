#include "StreamInput.h"

using namespace ThorsAnvil::Nisse::HTTP;

StreamBufInput::StreamBufInput(Complete&& complete)
    : remaining(0)
    , buffer(nullptr)
    , chunked(false)
    , firstChunk(false)
    , complete(std::move(complete))
{}

StreamBufInput::StreamBufInput(std::istream& stream, std::size_t length, Complete&& complete)
    : remaining(length)
    , buffer(stream.rdbuf())
    , chunked(false)
    , firstChunk(false)
    , complete(std::move(complete))
{}

StreamBufInput::StreamBufInput(std::istream& stream, Encoding /*encoding*/, Complete&& complete)
    : remaining(0)
    , buffer(stream.rdbuf())
    , chunked(true)
    , firstChunk(true)
    , complete(std::move(complete))
{}

StreamBufInput::StreamBufInput(StreamBufInput&& move) noexcept
    : remaining(std::exchange(move.remaining, 0))
    , buffer(std::exchange(move.buffer, nullptr))
    , chunked(std::exchange(move.chunked, false))
    , firstChunk(std::exchange(move.firstChunk, false))
    , complete(std::exchange(move.complete, [](){}))
{}

StreamBufInput& StreamBufInput::operator=(StreamBufInput&& move) noexcept
{
    remaining   = 0;
    buffer      = nullptr;
    chunked     = false;
    firstChunk  = false;
    complete    = [](){};

    swap(move);

    return *this;
}

void StreamBufInput::swap(StreamBufInput& other) noexcept
{
    std::streambuf::swap(other);

    using std::swap;
    swap(remaining, other.remaining);
    swap(buffer,    other.buffer);
    swap(chunked,   other.chunked);
    swap(firstChunk,other.firstChunk);
    swap(complete,  other.complete);
}

// Read:
StreamBufInput::int_type StreamBufInput::uflow()
{
    //std::cerr << "uflow\n";
    if (remaining == 0) {
        getNextChunk();
    }
    if (remaining == 0) {
        return traits::eof();
    }
    --remaining;
    int val = buffer->sbumpc();
    //std::cerr << "Got: " << val << " >" << static_cast<char>(val) << "<\n";
    return val;
}

std::streamsize StreamBufInput::xsgetn(char_type* s, std::streamsize count)
{
    //std::cerr << "\n\n\n==================\nxsgetn\n";
    //std::cerr << "\tRemaining: " << remaining << "  CK: " << chunked << " : " << count << "\n";
    std::streamsize result = 0;
    while (remaining != 0 || chunked)
    {
        std::streamsize nextChunk = std::min(count, remaining);
        std::streamsize got = buffer->sgetn(s, nextChunk);
        s           += got;
        count       -= got;
        result      += got;
        remaining   -= got;
        //std::cerr << "\tRead: " << nextChunk << " > " << got << "\n";
        if (count == 0) {
            break;
        }
        if (remaining == 0) {
            getNextChunk();
        }
    }
    return result;
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
