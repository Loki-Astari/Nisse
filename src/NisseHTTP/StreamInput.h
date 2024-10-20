#ifndef THORSANVIL_NISSE_NISSEHTTP_STREAMINPUT_H
#define THORSANVIL_NISSE_NISSEHTTP_STREAMINPUT_H

#include "NisseHTTPConfig.h"
#include "Util.h"
#include <iostream>

namespace ThorsAnvil::Nisse::NisseHTTP
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
        StreamBufInput(std::istream& stream, std::size_t length, Complete&& complete = [](){});
        StreamBufInput(std::istream& stream, Encoding encoding, Complete&& complete = [](){});
        StreamBufInput(StreamBufInput&& move)                   noexcept;
        StreamBufInput& operator=(StreamBufInput&& move)        noexcept;
        StreamBufInput(StreamBufInput const&)                   = delete;
        StreamBufInput& operator=(StreamBufInput const&)        = delete;

        void swap(StreamBufInput& other) noexcept;
        friend void swap(StreamBufInput& lhs, StreamBufInput& rhs)   {lhs.swap(rhs);}

        // Create directly from socket.
    protected:
        // Control:
        //virtual std::streambuf* setbuf(char_type* s, std::streamsize n)                 override {std::cerr << "setbuf\n";return buffer->pubsetbuf(s, n);}
        //virtual pos_type        seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
        //                                                                                override {std::cerr << "seekoff\n";return buffer->pubseekoff(off, dir, mode);}
        //virtual pos_type        seekpos(pos_type pos, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
        //                                                                                override {std::cerr << "seekpos\n";return buffer->pubseekpos(pos, mode);}
        //virtual int             sync() override;
        //virtual std::streamsize showmanyc()                                             override {std::cerr << "showmanyc\n";return 0;}
        //virtual int_type        underflow()                                             override {std::cerr << "underflow\n";return 'a';}

        // Read:
        virtual int_type        uflow() override;
        virtual std::streamsize xsgetn(char_type* s, std::streamsize count) override;

        // Write:
        //virtual std::streamsize xsputn(char_type const*,std::streamsize) override;
        //virtual int_type        overflow(int_type = traits::eof()) override;

        // Undo
        // virtual int_type        pbackfail(int_type c = traits::eof())                       override {std::cerr << "pbackfail\n";return 0;}

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
