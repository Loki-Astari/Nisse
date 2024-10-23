#ifndef THORSANVIL_NISSE_NISSEHTTP_STREAMINPUT_H
#define THORSANVIL_NISSE_NISSEHTTP_STREAMINPUT_H

#include "NisseHTTPConfig.h"
#include "Util.h"
#include <iostream>

namespace ThorsAnvil::Nisse::HTTP
{

class StreamBufInput: public std::streambuf
{
    public:
        using Complete = std::function<void()>;
        typedef std::streambuf::traits_type traits;
        typedef traits::int_type            int_type;
        typedef traits::char_type           char_type;
    private:
        std::streamsize     remaining;
        std::streambuf*     buffer;
        bool                chunked;
        bool                firstChunk;
        Complete            complete;
    public:
        StreamBufInput(Complete&& complete = [](){});
        StreamBufInput(std::istream& stream, std::streamsize length, Complete&& complete = [](){});
        StreamBufInput(std::istream& stream, Encoding encoding, Complete&& complete = [](){});
        StreamBufInput(StreamBufInput&& move)                   noexcept;
        StreamBufInput& operator=(StreamBufInput&& move)        noexcept;
        StreamBufInput(StreamBufInput const&)                   = delete;
        StreamBufInput& operator=(StreamBufInput const&)        = delete;

        void swap(StreamBufInput& other) noexcept;
        friend void swap(StreamBufInput& lhs, StreamBufInput& rhs)   {lhs.swap(rhs);}

        // Create directly from socket.
    protected:
        // Read:
        virtual int_type        uflow() override;
        virtual int_type        underflow() override;
        virtual std::streamsize xsgetn(char_type* s, std::streamsize count) override;
        virtual std::streamsize showmanyc() override;

    private:
        void checkBuffer();
        void getNextChunk();

};

class StreamInput: public std::istream
{
    StreamBufInput    buffer;
    public:
        StreamInput()
            : std::istream(nullptr)
            , buffer()
        {}
        StreamInput(std::istream& stream, std::size_t length)
            : std::istream(nullptr)
            , buffer(stream, length)
        {
            rdbuf(&buffer);
        }
        StreamInput(std::istream& stream, Encoding encoding)
            : std::istream(nullptr)
            , buffer(stream, encoding)
        {
            rdbuf(&buffer);
        }

        void addBuffer(StreamBufInput&& newBuffer)
        {
            buffer = std::move(newBuffer);
            rdbuf(&buffer);
            clear();
        }
};

}

#endif
