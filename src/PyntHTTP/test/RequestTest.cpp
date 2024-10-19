#include "gtest/gtest.h"

#include "Request.h"
#include <sstream>

using namespace ThorsAnvil::Nisse::PyntHTTP;

TEST(RequestTest, Construct)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Version::HTTP1_1, request.getVersion());
    EXPECT_EQ(Method::GET, request.getMethod());
    EXPECT_EQ(URL{"http://thorsanvil.dev:8080/Plop/path/twist.gue?p=1&q=12#34"}, request.getUrl());
    EXPECT_EQ("GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1", request.httpRawRequest());

    Header  expectedHeaders;
    expectedHeaders.add("host", "thorsanvil.dev:8080");
    EXPECT_EQ(expectedHeaders, request.headers());
}

TEST(RequestTest, HTTP1_0)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.0\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Version::HTTP1_0, request.getVersion());
}

TEST(RequestTest, HTTP1_1)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Version::HTTP1_1, request.getVersion());
}

TEST(RequestTest, HTTP2)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/2\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Version::HTTP2, request.getVersion());
}

TEST(RequestTest, HTTP3)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/3\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Version::HTTP3, request.getVersion());
}

TEST(RequestTest, Unknown)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 Loki/3\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Version::Unknown, request.getVersion());
}

TEST(RequestTest, GET)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::GET, request.getMethod());
}

TEST(RequestTest, HEAD)
{
    std::stringstream   stream{"HEAD /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::HEAD, request.getMethod());
}


TEST(RequestTest, OPTIONS)
{
    std::stringstream   stream{"OPTIONS /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::OPTIONS, request.getMethod());
}

TEST(RequestTest, TRACE)
{
    std::stringstream   stream{"TRACE /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::TRACE, request.getMethod());
}

TEST(RequestTest, PUT)
{
    std::stringstream   stream{"PUT /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::PUT, request.getMethod());
}

TEST(RequestTest, DELETE)
{
    std::stringstream   stream{"DELETE /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::DELETE, request.getMethod());
}

TEST(RequestTest, POST)
{
    std::stringstream   stream{"POST /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::POST, request.getMethod());
}

TEST(RequestTest, PATCH)
{
    std::stringstream   stream{"PATCH /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::PATCH, request.getMethod());
}

TEST(RequestTest, Other)
{
    std::stringstream   stream{"Loki /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::Other, request.getMethod());
}

TEST(RequestTest, CONNECT)
{
    std::stringstream   stream{"CONNECT /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::CONNECT, request.getMethod());
}

TEST(RequestTest, BadHeaderLine)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\n"
                               "transfer-encoding: chunked, gzip\n"
                               "transfer-encoding: zlib\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(1, request.headers().getHeader("host").size());
    EXPECT_EQ(3, request.headers().getHeader("transfer-encoding").size());
}

TEST(RequestTest, BadHeaderSplit)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "transfer-encoding: chunked, gzip\r\n"
                               "transfer-encoding: zlib\r\n"
                               "transferg zlib\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(1, request.headers().getHeader("host").size());
    EXPECT_EQ(3, request.headers().getHeader("transfer-encoding").size());
    EXPECT_EQ(0, request.headers().getHeader("transfer").size());
}

TEST(RequestTest, ContentLength)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\n"
                               "content-length: 36\r\n"
                               "\r\n"
                               "This is sime test\n"
                               "One more line XXX\n"
                               "Should not be able to read this"
                              };
    Request request("http", stream);

    std::string line;
    std::string expected[3] = {"This is sime test", "One more line XXX", "Protection"};
    int count = 0;
    while (std::getline(request.body(), line))
    {
        ASSERT_LT(count, 2);
        EXPECT_EQ(line ,expected[count]);
        ++count;
    }
    EXPECT_EQ(2, count);
}

TEST(RequestTest, ContentChunked)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\n"
                               "transfer-encoding: chunked\r\n"
                               "\r\n"
                               "12\r\nThis is sime test\n\r\n"
                               "12\r\nOne more line XXX\n\r\n"
                               "0\r\n"
                               "tail1: data\r\n"
                               "tail2: stop, loop\r\n"
                               "\r\n"
                               "This should be ignored"
                              };
    Request request("http", stream);

    std::string line;
    std::string expected[3] = {"This is sime test", "One more line XXX", "Protection"};
    int count = 0;
    while (std::getline(request.body(), line))
    {
        ASSERT_LT(count, 2);
        EXPECT_EQ(line ,expected[count]);
        ++count;
    }
    EXPECT_EQ(2, count);

    EXPECT_EQ(1, request.trailers().getHeader("tail1").size());
    EXPECT_EQ("data", request.trailers().getHeader("tail1")[0]);
    EXPECT_EQ(2, request.trailers().getHeader("tail2").size());
    EXPECT_EQ("stop", request.trailers().getHeader("tail2")[0]);
    EXPECT_EQ("loop", request.trailers().getHeader("tail2")[1]);
}


