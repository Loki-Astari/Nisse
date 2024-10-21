#include "gtest/gtest.h"
#include "PathMatcher.h"
#include "Request.h"
#include "Response.h"

using namespace ThorsAnvil::Nisse::NisseHTTP;


TEST(PathMatcherTest, Construct)
{
    PathMatcher         pm;
}

TEST(PathMatcherTest, SimplePathExactMatch)
{
    PathMatcher         pm;
    int                 count = 0;
    Match               hit;
    pm.addPath("/path1/path2/path3", [&count, &hit](Match const& match, Request&, Response&){++count;hit = match;});

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/path2/path3", request, response);

    EXPECT_EQ(1, count);
    EXPECT_EQ(0, hit.size());
}

TEST(PathMatcherTest, SimplePathExactMatchFailSuffix)
{
    PathMatcher         pm;
    int                 count = 0;
    Match               hit;
    pm.addPath("/path1/path2/path3", [&count, &hit](Match const& match, Request&, Response&){++count;hit = match;});

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/path2/path3more", request, response);

    EXPECT_EQ(0, count);
    EXPECT_EQ(0, hit.size());
}

TEST(PathMatcherTest, SimplePathExactMatchFailPrefix)
{
    PathMatcher         pm;
    int                 count = 0;
    Match               hit;
    pm.addPath("/path1/path2/path3", [&count, &hit](Match const& match, Request&, Response&){++count;hit = match;});

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("stuff/path1/path2/path3", request, response);

    EXPECT_EQ(0, count);
    EXPECT_EQ(0, hit.size());
}

TEST(PathMatcherTest, NameMatch)
{
    PathMatcher         pm;
    int                 count = 0;
    Match               hit;
    pm.addPath("/path1/{name}/path3", [&count, &hit](Match const& match, Request&, Response&){++count;hit = match;});

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/path2/path3", request, response);

    EXPECT_EQ(1, count);
    EXPECT_EQ(1, hit.size());
    EXPECT_EQ("name", hit.begin()->first);
    EXPECT_EQ("path2", hit.begin()->second);
}

TEST(PathMatcherTest, NameMatchSuffix)
{
    PathMatcher         pm;
    int                 count = 0;
    Match               hit;
    pm.addPath("/path1/{name}SUFFIX/path3", [&count, &hit](Match const& match, Request&, Response&){++count;hit = match;});

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/path2SUFFIX/path3", request, response);

    EXPECT_EQ(1, count);
    EXPECT_EQ(1, hit.size());
    EXPECT_EQ("name", hit.begin()->first);
    EXPECT_EQ("path2", hit.begin()->second);
}

TEST(PathMatcherTest, NameMatchPrefix)
{
    PathMatcher         pm;
    int                 count = 0;
    Match               hit;
    pm.addPath("/path1/PREFIX{name}/path3", [&count, &hit](Match const& match, Request&, Response&){++count;hit = match;});

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/PREFIXpath2/path3", request, response);

    EXPECT_EQ(1, count);
    EXPECT_EQ(1, hit.size());
    EXPECT_EQ("name", hit.begin()->first);
    EXPECT_EQ("path2", hit.begin()->second);
}

TEST(PathMatcherTest, NameMatchFail)
{
    PathMatcher         pm;
    int                 count = 0;
    Match               hit;
    pm.addPath("/path1/{name}/path3", [&count, &hit](Match const& match, Request&, Response&){++count;hit = match;});

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1A/path2/path3", request, response);

    EXPECT_EQ(0, count);
    EXPECT_EQ(0, hit.size());
}

TEST(PathMatcherTest, NameMatchSuffixFail)
{
    PathMatcher         pm;
    int                 count = 0;
    Match               hit;
    pm.addPath("/path1/{name}SUFFIX1/path3", [&count, &hit](Match const& match, Request&, Response&){++count;hit = match;});

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/path2SUFFIX/path3", request, response);

    EXPECT_EQ(0, count);
    EXPECT_EQ(0, hit.size());
}

TEST(PathMatcherTest, NameMatchPrefixFail)
{
    PathMatcher         pm;
    int                 count = 0;
    Match               hit;
    pm.addPath("/path1/PREFIX{name}/path3", [&count, &hit](Match const& match, Request&, Response&){++count;hit = match;});

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/1PREFIXpath2/path3", request, response);

    EXPECT_EQ(0, count);
    EXPECT_EQ(0, hit.size());
}

TEST(PathMatcherTest, NameMatchMultiple)
{
    PathMatcher         pm;
    int                 count = 0;
    Match               hit;
    pm.addPath("/path1/{name}/{id}", [&count, &hit](Match const& match, Request&, Response&){++count;hit = match;});

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/path2/path3", request, response);

    EXPECT_EQ(1, count);
    EXPECT_EQ(2, hit.size());
    EXPECT_EQ("name", (++hit.begin())->first);
    EXPECT_EQ("path2", (++hit.begin())->second);
    EXPECT_EQ("id", hit.begin()->first);
    EXPECT_EQ("path3", hit.begin()->second);
}


