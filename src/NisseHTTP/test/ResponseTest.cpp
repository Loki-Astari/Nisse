#include "gtest/gtest.h"
#include "Response.h"
#include "Util.h"
#include "HeaderResponse.h"
#include "HeaderPassThrough.h"
#include <sstream>
#include <ThorsLogging/ThorsLogging.h>

using namespace ThorsAnvil::Nisse::HTTP;

TEST(ResponseTest, Construct)
{
    std::stringstream   ss;
    {
        Response            response(ss, Version::HTTP1_1);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "content-length: 0\r\n"
              "\r\n", ss.str());
}

TEST(ResponseTest, ConstructNotStandard)
{
    std::stringstream   ss;
    {
        Response            response(ss, Version::HTTP1_1, 404);
    }

    EXPECT_EQ("HTTP/1.1 404 Not Found\r\n"
              "content-length: 0\r\n"
              "\r\n", ss.str());
}

TEST(ResponseTest, SetStateAfter)
{
    std::stringstream   ss;
    {
        Response            response(ss, Version::HTTP1_1);
        response.setStatus(511);
    }

    EXPECT_EQ("HTTP/1.1 511 Network Authentication Required\r\n"
              "content-length: 0\r\n"
              "\r\n", ss.str());
}

TEST(ResponseTest, AddLength)
{
    std::stringstream   ss;
    {
        Response            response(ss, Version::HTTP1_1);

        HeaderResponse      headers;
        response.addHeaders(headers);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "content-length: 0\r\n"
              "\r\n", ss.str());
}

TEST(ResponseTest, AddChunked)
{
    std::stringstream   ss;
    {
        Response            response(ss, Version::HTTP1_1);

        HeaderResponse      headers;
        response.addHeaders(headers);
        response.body(Encoding::Chunked);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "transfer-encoding: chunked\r\n"
              "\r\n"
              "0\r\n\r\n",
              ss.str());
}

TEST(ResponseTest, ZeroLengthWithHeader)
{
    std::stringstream   ss;
    {
        Response            response(ss, Version::HTTP1_1);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        response.addHeaders(headers);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "content-length: 0\r\n"
              "\r\n",
              ss.str());
}

TEST(ResponseTest, ZeroLengthWithHeaderContent)
{
    std::stringstream   ss;
    {
        Response            response(ss, Version::HTTP1_1);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        headers.add("content-length", "45");
        response.addHeaders(headers);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "content-length: 0\r\n"
              "\r\n",
              ss.str());
}

TEST(ResponseTest, ZeroLengthWithHeaderChunked)
{
    std::stringstream   ss;
    {
        Response            response(ss, Version::HTTP1_1);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        headers.add("transfer-encoding", "chunked");
        response.addHeaders(headers);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "content-length: 0\r\n"
              "\r\n",
              ss.str());
}

TEST(ResponseTest, FiveLengthWithHeader)
{
    std::stringstream   ss;
    {
        Response            response(ss, Version::HTTP1_1);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        response.addHeaders(headers);
        response.body(5) << "abcde";
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
        Response            response(ss, Version::HTTP1_1);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        headers.add("content-length", "45");
        response.addHeaders(headers);
        response.body(5) << "abcde";
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
        Response            response(ss, Version::HTTP1_1);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        headers.add("transfer-encoding", "chunked");
        response.addHeaders(headers);
        response.body(5) << "abcde";
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
        Response            response(ss, Version::HTTP1_1);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        response.addHeaders(headers);
        response.body(Encoding::Chunked);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "transfer-encoding: chunked\r\n"
              "\r\n"
              "0\r\n\r\n",
              ss.str());
}

TEST(ResponseTest, ZeroChunkedWithHeaderContent)
{
    std::stringstream   ss;
    {
        Response            response(ss, Version::HTTP1_1);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        headers.add("content-length", "45");
        response.addHeaders(headers);
        response.body(Encoding::Chunked);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "transfer-encoding: chunked\r\n"
              "\r\n"
              "0\r\n\r\n",
              ss.str());
}

TEST(ResponseTest, ZeroChunkedWithHeaderChunked)
{
    std::stringstream   ss;
    {
        Response            response(ss, Version::HTTP1_1);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        headers.add("transfer-encoding", "chunked");
        response.addHeaders(headers);
        response.body(Encoding::Chunked);
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "transfer-encoding: chunked\r\n"
              "\r\n"
              "0\r\n\r\n",
              ss.str());
}

TEST(ResponseTest, FiveChunkedWithHeader)
{
    std::stringstream   ss;
    {
        Response            response(ss, Version::HTTP1_1);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        response.addHeaders(headers);
        response.body(Encoding::Chunked) << "abcde";
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "transfer-encoding: chunked\r\n"
              "\r\n"
              "5\r\nabcde\r\n"
              "0\r\n\r\n",
              ss.str());
}

TEST(ResponseTest, FiveChunkedWithHeaderContent)
{
    std::stringstream   ss;
    {
        Response            response(ss, Version::HTTP1_1);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        headers.add("content-length", "45");
        response.addHeaders(headers);
        response.body(Encoding::Chunked) << "abcde";
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "transfer-encoding: chunked\r\n"
              "\r\n"
              "5\r\nabcde\r\n"
              "0\r\n\r\n",
              ss.str());
}

TEST(ResponseTest, FiveChunkedWithHeaderChunked)
{
    std::stringstream   ss;
    {
        Response            response(ss, Version::HTTP1_1);

        HeaderResponse      headers;
        headers.add("twist", "drive");
        headers.add("transfer-encoding", "chunked");
        response.addHeaders(headers);
        response.body(Encoding::Chunked) << "abcde";
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "twist: drive\r\n"
              "transfer-encoding: chunked\r\n"
              "\r\n"
              "5\r\nabcde\r\n"
              "0\r\n\r\n",
              ss.str());
}

TEST(ResponseTest, BodyNoHeaders)
{
    std::stringstream   ss;
    {
        Response            response(ss, Version::HTTP1_1);
        response.body(Encoding::Chunked) << "abcde";
    }

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "transfer-encoding: chunked\r\n"
              "\r\n"
              "5\r\nabcde\r\n"
              "0\r\n\r\n",
              ss.str());
}

TEST(ResponseTest, HeadersAfterBody)
{
    auto action = []()
    {
        std::stringstream   ss;
        {
            Response            response(ss, Version::HTTP1_1);

            HeaderResponse      headers;
            response.addHeaders(headers);
            response.body(Encoding::Chunked);
            response.addHeaders(headers);
        }
    };

    EXPECT_THROW(
        action(),
        std::runtime_error
    );
}

TEST(ResponseTest, MultiperHeadersOK)
{
    auto action = []()
    {
        std::stringstream   ss;
        {
            Response            response(ss, Version::HTTP1_1);

            HeaderResponse      headers;
            response.addHeaders(headers);
            response.addHeaders(headers);
            response.body(Encoding::Chunked);
        }
    };

    EXPECT_NO_THROW(
        action()
    );
}

