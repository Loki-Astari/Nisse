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

TEST(RequestTest, CONNECT)
{
    std::stringstream   stream{"CONNECT /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::CONNECT, request.getMethod());
}

