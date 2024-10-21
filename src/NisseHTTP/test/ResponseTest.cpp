#include "gtest/gtest.h"
#include "Response.h"
#include <sstream>
#include <ThorsLogging/ThorsLogging.h>

using namespace ThorsAnvil::Nisse::NisseHTTP;

TEST(ResponseTest, Construct)
{
    std::stringstream   ss;
    {
        Response            response(ss);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "\r\n", ss.str());
}

TEST(ResponseTest, ConstructNotStandard)
{
    std::stringstream   ss;
    {
        Response            response(ss, 404);
    }

    EXPECT_EQ("HTTP/1.1 404 Not Found\r\n"
              "\r\n", ss.str());
}

TEST(ResponseTest, SetStateAfter)
{
    std::stringstream   ss;
    {
        Response            response(ss);
        response.setStatus(511);
    }

    EXPECT_EQ("HTTP/1.1 511 Network Authentication Required\r\n"
              "\r\n", ss.str());
}

TEST(ResponseTest, AddLength)
{
    std::stringstream   ss;
    {
        Response            response(ss);

        HeaderResponse      headers;
        response.addHeaders(headers, 0);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "\r\n", ss.str());
}

TEST(ResponseTest, AddChunked)
{
    std::stringstream   ss;
    {
        Response            response(ss);

        HeaderResponse      headers;
        response.addHeaders(headers, Encoding::Chunked);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "transfer-encoding: chunked\r\n"
              "\r\n"
              "0\r\n",
              ss.str());
}

TEST(ResponseTest, ZeroLengthWithHeader)
{
    std::stringstream   ss;
    {
        Response            response(ss);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        response.addHeaders(headers, 0);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "\r\n",
              ss.str());
}

TEST(ResponseTest, ZeroLengthWithHeaderContent)
{
    std::stringstream   ss;
    {
        Response            response(ss);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        headers.add("content-length", "45");
        response.addHeaders(headers, 0);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "\r\n",
              ss.str());
}

TEST(ResponseTest, ZeroLengthWithHeaderChunked)
{
    std::stringstream   ss;
    {
        Response            response(ss);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        headers.add("transfer-encoding", "chunked");
        response.addHeaders(headers, 0);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "\r\n",
              ss.str());
}

TEST(ResponseTest, FiveLengthWithHeader)
{
    std::stringstream   ss;
    {
        Response            response(ss);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        response.addHeaders(headers, 5) << "abcde";
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "content-length: 5\r\n"
              "\r\n"
              "abcde",
              ss.str());
}

TEST(ResponseTest, FiveLengthWithHeaderContent)
{
    std::stringstream   ss;
    {
        Response            response(ss);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        headers.add("content-length", "45");
        response.addHeaders(headers, 5) << "abcde";
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "content-length: 5\r\n"
              "\r\n"
              "abcde",
              ss.str());
}

TEST(ResponseTest, FiveLengthWithHeaderChunked)
{
    std::stringstream   ss;
    {
        Response            response(ss);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        headers.add("transfer-encoding", "chunked");
        response.addHeaders(headers, 5) << "abcde";
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "content-length: 5\r\n"
              "\r\n"
              "abcde",
              ss.str());
}
/// ###
TEST(ResponseTest, ZeroChunkedWithHeader)
{
    std::stringstream   ss;
    {
        Response            response(ss);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        response.addHeaders(headers, Encoding::Chunked);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "transfer-encoding: chunked\r\n"
              "\r\n"
              "0\r\n",
              ss.str());
}

TEST(ResponseTest, ZeroChunkedWithHeaderContent)
{
    std::stringstream   ss;
    {
        Response            response(ss);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        headers.add("content-length", "45");
        response.addHeaders(headers, Encoding::Chunked);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "transfer-encoding: chunked\r\n"
              "\r\n"
              "0\r\n",
              ss.str());
}

TEST(ResponseTest, ZeroChunkedWithHeaderChunked)
{
    std::stringstream   ss;
    {
        Response            response(ss);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        headers.add("transfer-encoding", "chunked");
        response.addHeaders(headers, Encoding::Chunked);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "transfer-encoding: chunked\r\n"
              "\r\n"
              "0\r\n",
              ss.str());
}

TEST(ResponseTest, FiveChunkedWithHeader)
{
    std::stringstream   ss;
    {
        Response            response(ss);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        response.addHeaders(headers, Encoding::Chunked) << "abcde";
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "transfer-encoding: chunked\r\n"
              "\r\n"
              "5\r\nabcde\r\n"
              "0\r\n",
              ss.str());
}

TEST(ResponseTest, FiveChunkedWithHeaderContent)
{
    std::stringstream   ss;
    {
        Response            response(ss);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        headers.add("content-length", "45");
        response.addHeaders(headers, Encoding::Chunked) << "abcde";
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "transfer-encoding: chunked\r\n"
              "\r\n"
              "5\r\nabcde\r\n"
              "0\r\n",
              ss.str());
}

TEST(ResponseTest, FiveChunkedWithHeaderChunked)
{
    std::stringstream   ss;
    {
        Response            response(ss);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        headers.add("transfer-encoding", "chunked");
        response.addHeaders(headers, Encoding::Chunked) << "abcde";
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "transfer-encoding: chunked\r\n"
              "\r\n"
              "5\r\nabcde\r\n"
              "0\r\n",
              ss.str());
}

TEST(ResponseTest, HeadersHeaders)
{
    auto action = []()
    {
        std::stringstream   ss;
        {
            Response            response(ss);

            HeaderResponse      headers;
            response.addHeaders(headers, Encoding::Chunked);
            response.addHeaders(headers, Encoding::Chunked);
        }
    };

    EXPECT_THROW(
        action(),
        ThorsAnvil::Logging::LogicalException
    );
}



#if 0
class Response
{
    Version         version;
    StatusCode      statusCode;
    bool            headerSent;
    bool            bodySent;

    std::ostream&   baseStream;
    StreamOutput    stream;

    public:
        Response(std::ostream& stream, Version version, int code = 200);
        void                setStatus(int code);

        std::ostream&       addHeaders(HeaderResponse const& headers, Encoding type);
        std::ostream&       addHeaders(HeaderResponse const& headers, std::size_t length);
    private:
        std::ostream&       addHeaders(HeaderResponse const& headers, StreamBufOutput&& buffer, std::string_view extraHeader);
};

#endif
