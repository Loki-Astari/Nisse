#include "gtest/gtest.h"
#include "PathMatcher.h"
#include "Request.h"
#include "Response.h"

using namespace ThorsAnvil::Nisse::HTTP;


TEST(PathMatcherTest, Construct)
{
    PathMatcher         pm;
}

struct TestObject
{
    int                 count = 0;
    Match               hit;
};
struct TestData: public PathMatcher::Data
{
    TestData(TestObject& object)
        : PathMatcher::Data(nullptr, "", [](Request const&, Response&){return true;}, [](Request const&){return true;})
        , object(object)
    {}
    TestObject&         object;
};
TEST(PathMatcherTest, SimplePathExactMatch)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1/path2/path3",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    ++td.object.count;
                    td.object.hit = match;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/path2/path3", request, response);

    EXPECT_EQ(1, object.count);
    EXPECT_EQ(0, object.hit.size());
}

TEST(PathMatcherTest, SimplePathExactMatchFailSuffix)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1/path2/path3",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    ++td.object.count;
                    td.object.hit = match;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));
    int                 count = 0;
    Match               hit;

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/path2/path3more", request, response);

    EXPECT_EQ(0, object.count);
    EXPECT_EQ(0, object.hit.size());
}

TEST(PathMatcherTest, SimplePathExactMatchFailPrefix)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1/path2/path3",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    ++td.object.count;
                    td.object.hit = match;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("stuff/path1/path2/path3", request, response);

    EXPECT_EQ(0, object.count);
    EXPECT_EQ(0, object.hit.size());
}

TEST(PathMatcherTest, NameMatch)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1/{name}/path3",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    ++td.object.count;
                    td.object.hit = match;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/path2/path3", request, response);

    EXPECT_EQ(1, object.count);
    EXPECT_EQ(1, object.hit.size());
    EXPECT_EQ("name", object.hit.begin()->first);
    EXPECT_EQ("path2", object.hit.begin()->second);
}

TEST(PathMatcherTest, NameMatchSuffix)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1/{name}SUFFIX/path3",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    ++td.object.count;
                    td.object.hit = match;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/path2SUFFIX/path3", request, response);

    EXPECT_EQ(1, object.count);
    EXPECT_EQ(1, object.hit.size());
    EXPECT_EQ("name", object.hit.begin()->first);
    EXPECT_EQ("path2", object.hit.begin()->second);
}

TEST(PathMatcherTest, NameMatchPrefix)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1/PREFIX{name}/path3",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    ++td.object.count;
                    td.object.hit = match;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/PREFIXpath2/path3", request, response);

    EXPECT_EQ(1, object.count);
    EXPECT_EQ(1, object.hit.size());
    EXPECT_EQ("name", object.hit.begin()->first);
    EXPECT_EQ("path2", object.hit.begin()->second);
}

TEST(PathMatcherTest, NameMatchFail)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1/{name}/path3",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    ++td.object.count;
                    td.object.hit = match;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1A/path2/path3", request, response);

    EXPECT_EQ(0, object.count);
    EXPECT_EQ(0, object.hit.size());
}

TEST(PathMatcherTest, NameMatchSuffixFail)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1/{name}SUFFIX1/path3",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    ++td.object.count;
                    td.object.hit = match;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/path2SUFFIX/path3", request, response);

    EXPECT_EQ(0, object.count);
    EXPECT_EQ(0, object.hit.size());
}

TEST(PathMatcherTest, NameMatchPrefixFail)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1/PREFIX{name}/path3",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    ++td.object.count;
                    td.object.hit = match;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/1PREFIXpath2/path3", request, response);

    EXPECT_EQ(0, object.count);
    EXPECT_EQ(0, object.hit.size());
}

TEST(PathMatcherTest, NameMatchMultiple)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1/{name}/{id}",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    ++td.object.count;
                    td.object.hit = match;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/path2/path3", request, response);

    EXPECT_EQ(1, object.count);
    EXPECT_EQ(2, object.hit.size());
    EXPECT_EQ("name", (++object.hit.begin())->first);
    EXPECT_EQ("path2", (++object.hit.begin())->second);
    EXPECT_EQ("id", object.hit.begin()->first);
    EXPECT_EQ("path3", object.hit.begin()->second);
}

TEST(PathMatcherTest, NameMatchEverything)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "{all}",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    ++td.object.count;
                    td.object.hit = match;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/path2/path3", request, response);

    EXPECT_EQ(1, object.count);
    EXPECT_EQ(1, object.hit.size());
    EXPECT_EQ("all", object.hit.begin()->first);
    EXPECT_EQ("/path1/path2/path3", object.hit.begin()->second);
}

TEST(PathMatcherTest, NameMatchFront)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "{prefix}/path3",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    ++td.object.count;
                    td.object.hit = match;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/path2/path3", request, response);

    EXPECT_EQ(1, object.count);
    EXPECT_EQ(1, object.hit.size());
    EXPECT_EQ("prefix", object.hit.begin()->first);
    EXPECT_EQ("/path1/path2", object.hit.begin()->second);
}

TEST(PathMatcherTest, NameMatchBack)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1{suffix}",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    ++td.object.count;
                    td.object.hit = match;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    pm.findMatch("/path1/path2/path3", request, response);

    EXPECT_EQ(1, object.count);
    EXPECT_EQ(1, object.hit.size());
    EXPECT_EQ("suffix", object.hit.begin()->first);
    EXPECT_EQ("/path2/path3", object.hit.begin()->second);
}


TEST(PathMatcherTest, ExactPathWillOverrideOriginal)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1{suffix}",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    td.object.count += 1;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));
    pm.addPath(Method::GET,
               "/path1{suffix}",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    td.object.count += 2;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    bool        hit = pm.findMatch("/path1/path2/path3", request, response);

    EXPECT_TRUE(hit);
    EXPECT_EQ(2, object.count);
}

TEST(PathMatcherTest, NotExactPathWillHitFirst)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1{suffix}",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    td.object.count += 1;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));
    pm.addPath(Method::GET,
               "/path1/{suffix}",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    td.object.count += 2;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    bool        hit = pm.findMatch("/path1/path2/path3", request, response);

    EXPECT_TRUE(hit);
    EXPECT_EQ(1, object.count);
}

TEST(PathMatcherTest, NotExactPathWillHitFirstAndFallThroughToSecond)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1{suffix}",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    td.object.count += 1;
                    return false;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));
    pm.addPath(Method::GET,
               "/path1/{suffix}",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    td.object.count += 2;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    bool        hit = pm.findMatch("/path1/path2/path3", request, response);

    EXPECT_TRUE(hit);
    EXPECT_EQ(3, object.count);
}

TEST(PathMatcherTest, NotExactPathWillHitFirstAndFallThrough)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1{suffix}",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    td.object.count += 1;
                    return false;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));
    pm.addPath(Method::GET,
               "/path1/{suffix}",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    td.object.count += 2;
                    return false;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    bool        hit = pm.findMatch("/path1/path2/path3", request, response);

    EXPECT_FALSE(hit);
    EXPECT_EQ(3, object.count);
}

TEST(PathMatcherTest, PathRemoved)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1{suffix}",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    td.object.count += 1;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));
    pm.remPath(Method::GET, "/path1{suffix}");

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    bool        hit = pm.findMatch("/path1/path2/path3", request, response);

    EXPECT_FALSE(hit);
    EXPECT_EQ(0, object.count);
}

TEST(PathMatcherTest, PathRemovedFirst)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1{suffix}",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    td.object.count += 1;
                    return false;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));
    pm.addPath(Method::GET,
               "/path1/{suffix}",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    td.object.count += 2;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));
    pm.remPath(Method::GET, "/path1{suffix}");

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    bool        hit = pm.findMatch("/path1/path2/path3", request, response);

    EXPECT_TRUE(hit);
    EXPECT_EQ(2, object.count);
}

TEST(PathMatcherTest, PathRemovedSecond)
{
    PathMatcher         pm;
    TestObject          object;
    pm.addPath(Method::GET,
               "/path1{suffix}",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    td.object.count += 1;
                    return false;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));
    pm.addPath(Method::GET,
               "/path1/{suffix}",
               [](PathMatcher::Data& data, Match const& match, Request&, Response&)
               {
                    TestData&  td = dynamic_cast<TestData&>(data);
                    td.object.count += 2;
                    return true;
               },
               std::unique_ptr<PathMatcher::Data>(new TestData(object)));
    pm.remPath(Method::GET, "/path1/{suffix}");

    std::stringstream   ss{"GET /path1/path2/path3 HTTP/1.1\r\nhost: google.com\r\n\r\n"};
    Request     request("http", ss);
    Response    response(ss, Version::HTTP1_1);
    bool        hit = pm.findMatch("/path1/path2/path3", request, response);

    EXPECT_FALSE(hit);
    EXPECT_EQ(1, object.count);
}


