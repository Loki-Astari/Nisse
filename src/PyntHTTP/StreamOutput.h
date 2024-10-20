#ifndef THORSANVIL_NISSE_HTTP_STREAMOUTPUT_H
#define THORSANVIL_NISSE_HTTP_STREAMOUTPUT_H

#include "HTTPConfig.h"
#include "Util.h"
#include <iostream>

namespace ThorsAnvil::Nisse::HTTP
{

class StreamBufOutput: public std::streambuf
{
    public:
        using Complete = std::function<void()>;
        typedef std::streambuf::traits_type traits;
        typedef traits::int_type            int_type;
        typedef traits::char_type           char_type;
    private:
        static std::streamsize constexpr chunkBufferSize = 1024;
        std::streamsize     remaining;
        std::streambuf*     buffer;
        bool                chunked;
        bool                firstChunk;
        Complete            complete;
        std::vector<char>   chunkBuffer;
    public:
        ~StreamBufOutput();
        StreamBufOutput(Complete&& complete = [](){});
        StreamBufOutput(std::ostream& stream, std::size_t length, Complete&& complete = [](){});
        StreamBufOutput(std::ostream& stream, Encoding encoding, Complete&& complete = [](){});
        StreamBufOutput(StreamBufOutput&& move)                   noexcept;
        StreamBufOutput& operator=(StreamBufOutput&& move)        noexcept;
        StreamBufOutput(StreamBufOutput const&)                   = delete;
        StreamBufOutput& operator=(StreamBufOutput const&)        = delete;

        void swap(StreamBufOutput& other) noexcept;
        friend void swap(StreamBufOutput& lhs, StreamBufOutput& rhs)   {lhs.swap(rhs);}

        void done();

        // Create directly from socket.
    protected:
        // Control:
        //virtual std::streambuf* setbuf(char_type* s, std::streamsize n)                 override {std::cerr << "setbuf\n";return buffer->pubsetbuf(s, n);}
        //virtual pos_type        seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
        //                                                                                override {std::cerr << "seekoff\n";return buffer->pubseekoff(off, dir, mode);}
        //virtual pos_type        seekpos(pos_type pos, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
        //                                                                                override {std::cerr << "seekpos\n";return buffer->pubseekpos(pos, mode);}
        virtual int             sync() override;
        //virtual std::streamsize showmanyc()                                             override {std::cerr << "showmanyc\n";return 0;}
        //virtual int_type        underflow()                                             override {std::cerr << "underflow\n";return 'a';}

        // Read:
        //virtual int_type        uflow() override;
        //virtual std::streamsize xsgetn(char_type* s, std::streamsize count) override;

        // Write:
        virtual std::streamsize xsputn(char_type const*,std::streamsize) override;
        virtual int_type        overflow(int_type = traits::eof()) override;

        // Undo
        // virtual int_type        pbackfail(int_type c = traits::eof())                       override {std::cerr << "pbackfail\n";return 0;}

    private:
        void checkBuffer();
        void outputChunkSize(std::streamsize size);
        char toHex(int digit);
        void sendAllData(char const* s, std::streamsize size);

        std::streamsize xsputnChunked(char_type const*,std::streamsize);
        std::streamsize xsputnLength(char_type const*,std::streamsize);
        int_type        overflowChunked(int_type = traits::eof());
        int_type        overflowLength(int_type = traits::eof());
        void            dumpBuffer();
};

class StreamOutput: public std::ostream
{
    StreamBufOutput    buffer;
    public:
        StreamOutput()
            : std::ostream(nullptr)
            , buffer()
        {}
        StreamOutput(std::ostream& stream, std::size_t length)
            : std::ostream(nullptr)
            , buffer(stream, length)
        {
            rdbuf(&buffer);
        }
        StreamOutput(std::ostream& stream, Encoding encoding)
            : std::ostream(nullptr)
            , buffer(stream, encoding)
        {
            rdbuf(&buffer);
        }

        void addBuffer(StreamBufOutput&& newBuffer)
        {
            buffer = std::move(newBuffer);
            rdbuf(&buffer);
            clear();
        }
};

}

#endif
