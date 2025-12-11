#include "gtest/gtest.h"
#include "HTTPHandler.h"
#include "Request.h"
#include "Response.h"

using namespace ThorsAnvil::Nisse::HTTP;

TEST(HTTPHandlerTest, Construct)
{
    HTTPHandler         handler;
}

TEST(HTTPHandlerTest, SimplePathExactMatch)
{
    HTTPHandler         httpHandler;
    int                 count = 0;
    httpHandler.addPath(Method::GET, "/path1/path2/path3", [&count](Request&, Response&){++count;return true;});

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\n"
                           "host: google.com\r\n"
                           "\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    httpHandler.processRequest(request, response);

    EXPECT_EQ(1, count);
    EXPECT_EQ(1, request.variables().size());
    EXPECT_EQ("google.com", request.variables()["host"]);
}

TEST(HTTPHandlerTest, SimplePathExactMatchWithQuery)
{
    HTTPHandler         httpHandler;
    int                 count = 0;
    httpHandler.addPath(Method::GET, "/path1/path2/path3", [&count](Request&, Response&){++count;return true;});

    std::stringstream   ss{"GET /path1/path2/path3?user=Loki HTTP/1.1\r\n"
                           "host: google.com\r\n"
                           "\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    httpHandler.processRequest(request, response);

    EXPECT_EQ(1, count);
    EXPECT_EQ(2, request.variables().size());
    EXPECT_EQ("google.com", request.variables()["host"]);
    EXPECT_EQ("Loki", request.variables()["user"]);
}

TEST(HTTPHandlerTest, SimplePathExactMatchQueryOverride)
{
    HTTPHandler         httpHandler;
    int                 count = 0;
    httpHandler.addPath(Method::GET, "/path1/path2/path3", [&count](Request&, Response&){++count;return true;});

    std::stringstream   ss{"GET /path1/path2/path3?host=twitter.com HTTP/1.1\r\n"
                           "host: google.com\r\n"
                           "\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    httpHandler.processRequest(request, response);

    EXPECT_EQ(1, count);
    EXPECT_EQ(1, request.variables().size());
    EXPECT_EQ("twitter.com", request.variables()["host"]);
}


TEST(HTTPHandlerTest, PathMatch)
{
    HTTPHandler         httpHandler;
    int                 count = 0;
    httpHandler.addPath(Method::GET, "/path1/{name}/path3", [&count](Request&, Response&){++count;return true;});

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\n"
                           "host: google.com\r\n"
                           "\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    httpHandler.processRequest(request, response);

    EXPECT_EQ(1, count);
    EXPECT_EQ(2, request.variables().size());
    EXPECT_EQ("google.com", request.variables()["host"]);
    EXPECT_EQ("path2", request.variables()["name"]);
}

TEST(HTTPHandlerTest, PathMatchWithQuery)
{
    HTTPHandler         httpHandler;
    int                 count = 0;
    httpHandler.addPath(Method::GET, "/path1/{name}/path3", [&count](Request&, Response&){++count;return true;});

    std::stringstream   ss{"GET /path1/path2/path3?user=Loki HTTP/1.1\r\n"
                           "host: google.com\r\n"
                           "\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    httpHandler.processRequest(request, response);

    EXPECT_EQ(1, count);
    EXPECT_EQ(3, request.variables().size());
    EXPECT_EQ("google.com", request.variables()["host"]);
    EXPECT_EQ("Loki", request.variables()["user"]);
    EXPECT_EQ("path2", request.variables()["name"]);
}

TEST(HTTPHandlerTest, PathMatchQueryOverride)
{
    HTTPHandler         httpHandler;
    int                 count = 0;
    httpHandler.addPath(Method::GET, "/path1/{name}/path3", [&count](Request&, Response&){++count;return true;});

    std::stringstream   ss{"GET /path1/path2/path3?host=twitter.com HTTP/1.1\r\n"
                           "host: google.com\r\n"
                           "\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    httpHandler.processRequest(request, response);

    EXPECT_EQ(1, count);
    EXPECT_EQ(2, request.variables().size());
    EXPECT_EQ("twitter.com", request.variables()["host"]);
    EXPECT_EQ("path2", request.variables()["name"]);
}

TEST(HTTPHandlerTest, PathMatchMAtchOverride)
{
    HTTPHandler         httpHandler;
    int                 count = 0;
    httpHandler.addPath(Method::GET, "/path1/{host}/path3", [&count](Request&, Response&){++count;return true;});

    std::stringstream   ss{"GET /path1/path2/path3?host=twitter.com HTTP/1.1\r\n"
                           "host: google.com\r\n"
                           "\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    httpHandler.processRequest(request, response);

    EXPECT_EQ(1, count);
    EXPECT_EQ(1, request.variables().size());
    EXPECT_EQ("path2", request.variables()["host"]);
}

TEST(HTTPHandlerTest, FormParameterPassed)
{
    HTTPHandler         httpHandler;
    int                 count = 0;
    httpHandler.addPath(Method::GET, "/path1/{host}/path3", [&count](Request&, Response&){++count;return true;});

    std::stringstream   ss{"GET /path1/path2/path3?host=twitter.com HTTP/1.1\r\n"
                           "host: google.com\r\n"
                           "content-type: application/x-www-form-urlencoded\r\n"
                           "content-length: 56\r\n"
                           "\r\n"
                           "Name=Loki&Address=12345+address&funny=%20%26%23%21+Stuff"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    httpHandler.processRequest(request, response);

    EXPECT_EQ("Loki", request.variables()["Name"]);
    EXPECT_EQ("12345 address", request.variables()["Address"]);
    EXPECT_EQ(" &#! Stuff", request.variables()["funny"]);
}

TEST(HTTPHandlerTest, TwoRegisteredHitsFirstOnly)
{
    HTTPHandler         httpHandler;
    int                 validate = 0;
    httpHandler.addPath(Method::GET, "/path1{host}/path3", [&validate](Request&, Response&){validate += 1;return true;});
    httpHandler.addPath(Method::GET, "/path1/{host}/path3", [&validate](Request&, Response&){validate += 2;return true;});

    std::stringstream   ss{"GET /path1/path2/path3?host=twitter.com HTTP/1.1\r\n"
                           "host: google.com\r\n"
                           "content-type: application/x-www-form-urlencoded\r\n"
                           "content-length: 56\r\n"
                           "\r\n"
                           "Name=Loki&Address=12345+address&funny=%20%26%23%21+Stuff"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    httpHandler.processRequest(request, response);

    EXPECT_EQ("Loki", request.variables()["Name"]);
    EXPECT_EQ("12345 address", request.variables()["Address"]);
    EXPECT_EQ(" &#! Stuff", request.variables()["funny"]);
    EXPECT_EQ(1, validate);
}

TEST(HTTPHandlerTest, TwoRegisteredHitsSecondFirstRetunsFalse)
{
    HTTPHandler         httpHandler;
    int                 validate = 0;
    httpHandler.addPath(Method::GET, "/path1{host}/path3", [&validate](Request&, Response&){validate += 1;return false;});
    httpHandler.addPath(Method::GET, "/path1/{host}/path3", [&validate](Request&, Response&){validate += 2;return true;});

    std::stringstream   ss{"GET /path1/path2/path3?host=twitter.com HTTP/1.1\r\n"
                           "host: google.com\r\n"
                           "content-type: application/x-www-form-urlencoded\r\n"
                           "content-length: 56\r\n"
                           "\r\n"
                           "Name=Loki&Address=12345+address&funny=%20%26%23%21+Stuff"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    httpHandler.processRequest(request, response);

    EXPECT_EQ("Loki", request.variables()["Name"]);
    EXPECT_EQ("12345 address", request.variables()["Address"]);
    EXPECT_EQ(" &#! Stuff", request.variables()["funny"]);
    EXPECT_EQ(3, validate);
}

TEST(HTTPHandlerTest, TwoRegisteredHitsSecondFirstRemoved)
{
    HTTPHandler         httpHandler;
    int                 validate = 0;
    httpHandler.addPath(Method::GET, "/path1{host}/path3", [&validate](Request&, Response&){validate += 1;return true;});
    httpHandler.addPath(Method::GET, "/path1/{host}/path3", [&validate](Request&, Response&){validate += 2;return true;});
    httpHandler.remPath(Method::GET, "/path1{host}/path3");

    std::stringstream   ss{"GET /path1/path2/path3?host=twitter.com HTTP/1.1\r\n"
                           "host: google.com\r\n"
                           "content-type: application/x-www-form-urlencoded\r\n"
                           "content-length: 56\r\n"
                           "\r\n"
                           "Name=Loki&Address=12345+address&funny=%20%26%23%21+Stuff"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    httpHandler.processRequest(request, response);

    EXPECT_EQ("Loki", request.variables()["Name"]);
    EXPECT_EQ("12345 address", request.variables()["Address"]);
    EXPECT_EQ(" &#! Stuff", request.variables()["funny"]);
    EXPECT_EQ(2, validate);
}

TEST(HTTPHandlerTest, TwoRegisteredHitsFirstAfterReplacement)
{
    HTTPHandler         httpHandler;
    int                 validate = 0;
    httpHandler.addPath(Method::GET, "/path1{host}/path3", [&validate](Request&, Response&){validate += 1;return true;});
    httpHandler.addPath(Method::GET, "/path1/{host}/path3", [&validate](Request&, Response&){validate += 2;return true;});
    httpHandler.remPath(Method::GET, "/path1{host}/path3");
    httpHandler.addPath(Method::GET, "/path1{host}/path3", [&validate](Request&, Response&){validate += 4;return true;});

    std::stringstream   ss{"GET /path1/path2/path3?host=twitter.com HTTP/1.1\r\n"
                           "host: google.com\r\n"
                           "content-type: application/x-www-form-urlencoded\r\n"
                           "content-length: 56\r\n"
                           "\r\n"
                           "Name=Loki&Address=12345+address&funny=%20%26%23%21+Stuff"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    httpHandler.processRequest(request, response);

    EXPECT_EQ("Loki", request.variables()["Name"]);
    EXPECT_EQ("12345 address", request.variables()["Address"]);
    EXPECT_EQ(" &#! Stuff", request.variables()["funny"]);
    EXPECT_EQ(4, validate);
}


