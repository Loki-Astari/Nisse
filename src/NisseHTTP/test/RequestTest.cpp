#include "gtest/gtest.h"

#include "Request.h"
#include <sstream>

using namespace ThorsAnvil::Nisse::HTTP;

TEST(RequestTest, Construct)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Version::HTTP1_1, request.getVersion());
    EXPECT_EQ(Method::GET, request.getMethod());
    EXPECT_EQ(URL{"http://thorsanvil.dev:8080/Plop/path/twist.gue?p=1&q=12#34"}, request.getUrl());
    EXPECT_EQ("GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1", request.httpRawRequest());

    //Header  expectedHeaders;
    //expectedHeaders.add("host", "thorsanvil.dev:8080");
    //EXPECT_EQ(expectedHeaders, request.headers());
}

TEST(RequestTest, HTTP1_0)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.0\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Version::HTTP1_0, request.getVersion());
}

TEST(RequestTest, HTTP1_1)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Version::HTTP1_1, request.getVersion());
}

TEST(RequestTest, HTTP2)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/2\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Version::HTTP2, request.getVersion());
}

TEST(RequestTest, HTTP3)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/3\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Version::HTTP3, request.getVersion());
}

TEST(RequestTest, Unknown)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 Loki/3\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_FALSE(request.isValidRequest());
}

TEST(RequestTest, GET)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::GET, request.getMethod());
}

TEST(RequestTest, HEAD)
{
    std::stringstream   stream{"HEAD /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::HEAD, request.getMethod());
}


TEST(RequestTest, OPTIONS)
{
    std::stringstream   stream{"OPTIONS /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::OPTIONS, request.getMethod());
}

TEST(RequestTest, TRACE)
{
    std::stringstream   stream{"TRACE /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::TRACE, request.getMethod());
}

TEST(RequestTest, PUT)
{
    std::stringstream   stream{"PUT /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::PUT, request.getMethod());
}

TEST(RequestTest, DELETER)
{
    std::stringstream   stream{"DELETE /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::DELETER, request.getMethod());
}

TEST(RequestTest, POST)
{
    std::stringstream   stream{"POST /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::POST, request.getMethod());
}

TEST(RequestTest, PATCH)
{
    std::stringstream   stream{"PATCH /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::PATCH, request.getMethod());
}

TEST(RequestTest, Other)
{
    std::stringstream   stream{"Loki /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_FALSE(request.isValidRequest());
}

TEST(RequestTest, CONNECT)
{
    std::stringstream   stream{"CONNECT /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_EQ(Method::CONNECT, request.getMethod());
}

TEST(RequestTest, BadMessageHeaderNotTerminated)
{
    std::stringstream   stream{"GET HTTP/1.1\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "someheader: value1, value2\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_FALSE(request.isValidRequest());
}

TEST(RequestTest, BadMissingField)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "someheader: value1, value2\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_FALSE(request.isValidRequest());
}

TEST(RequestTest, BadHeaderLine)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\n"
                               "content-length: 0\r\n"
                               "someheader: value1, value2\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_FALSE(request.isValidRequest());
}

TEST(RequestTest, BadHeaderSplit)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "content-length: 0\r\n"
                               "someheader; value1, value2\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_FALSE(request.isValidRequest());
}

TEST(RequestTest, MissingHost)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "content-length: 0\r\n"
                               "someheader: value1, value2\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    EXPECT_FALSE(request.isValidRequest());
}

TEST(RequestTest, MissingContentSize)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
                               "someheader: value1, value2\r\n"
                               "\r\n"
                              };
    Request request("http", stream);

    // No content-length is OK.
    // If no encoding and no content-length the body is 0 bytes

    EXPECT_TRUE(request.isValidRequest());
}

TEST(RequestTest, ContentLength)
{
    std::stringstream   stream{"GET /Plop/path/twist.gue?p=1&q=12#34 HTTP/1.1\r\n"
                               "host: thorsanvil.dev:8080\r\n"
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
                               "host: thorsanvil.dev:8080\r\n"
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

TEST(RequestTest, Stream)
{
    std::stringstream   ss{"GET /Path1/Path2/Stop.html HTTP/1.1\r\n"
                           "host: boimler.com\r\n"
                           "header1: stuff\r\n"
                           "header2  : pump\r\n"
                           "header2: loop\r\n"
                           "content-length: 20\r\n"
                           "\r\n"
                           "12345678901234567890"
                           "--------------------"
                          };
    Request             request{"https", ss};

    std::stringstream   out;
    out << request
        << request.body().rdbuf()
        << std::flush;

    EXPECT_EQ(out.str(),
              "GET /Path1/Path2/Stop.html HTTP/1.1\r\n"
              "content-length: 20\r\n"
              "header1: stuff\r\n"
              "header2: pump, loop\r\n"
              "host: boimler.com\r\n"
              "\r\n"
              "12345678901234567890"
             );
}

TEST(RequestTest, BadBodyInformation)
{
    std::stringstream   ss{"GET /Path1/Path2/Stop.html HTTP/1.1\r\n"
                           "host: boimler.com\r\n"
                           "content-length: 20\r\n"
                           "transfer-encoding: chunked\r\n"
                           "header1: stuff\r\n"
                           "header2  : pump\r\n"
                           "header2: loop\r\n"
                           "\r\n"
                           "12345678901234567890"
                           "--------------------"
                          };
    Request             request{"https", ss};
    EXPECT_FALSE(request.isValidRequest());
    EXPECT_FALSE(request.failHeader().empty());
}

TEST(RequestTest, UnsoportTransferEncodingInformation)
{
    std::stringstream   ss{"GET /Path1/Path2/Stop.html HTTP/1.1\r\n"
                           "host: boimler.com\r\n"
                           "transfer-encoding: long\r\n"
                           "header1: stuff\r\n"
                           "header2  : pump\r\n"
                           "header2: loop\r\n"
                           "\r\n"
                           "12345678901234567890"
                           "--------------------"
                          };
    Request             request{"https", ss};
    EXPECT_FALSE(request.isValidRequest());
    EXPECT_FALSE(request.failHeader().empty());
}
