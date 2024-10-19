#include "gtest/gtest.h"

#include "Util.h"

TEST(HeaderTest, Costruct)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header{};
}

TEST(HeaderTest, AddHeadderStringLiteral)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("Content-Length", "56");
    using std::literals::operator""sv;
    EXPECT_EQ(1, header.getHeader("content-length"sv).size());
    EXPECT_EQ("56", header.getHeader("content-length"sv)[0]);

    EXPECT_EQ(0, header.getHeader("Content-Length"sv).size());
}

TEST(HeaderTest, AddHeadderString)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("Content-Length", "56");
    using std::literals::operator""s;
    EXPECT_EQ(1, header.getHeader("content-length"s).size());
    EXPECT_EQ("56", header.getHeader("content-length"s)[0]);

    EXPECT_EQ(0, header.getHeader("Content-Length"s).size());
}

TEST(HeaderTest, AddHeadderCharArray)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("Content-Length", "56");
    EXPECT_EQ(1, header.getHeader("content-length").size());
    EXPECT_EQ("56", header.getHeader("content-length")[0]);

    EXPECT_EQ(0, header.getHeader("Content-Length").size());
}

TEST(HeaderTest, AddHeadderConstStringLiteral)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     headerBase;
    ThorsAnvil::Nisse::PyntHTTP::Header const& header = headerBase;

    headerBase.add("Content-Length", "56");
    using std::literals::operator""sv;
    EXPECT_EQ(1, header.getHeader("content-length"sv).size());
    EXPECT_EQ("56", header.getHeader("content-length"sv)[0]);

    EXPECT_EQ(0, header.getHeader("Content-Length"sv).size());
}

TEST(HeaderTest, AddHeadderConstString)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     headerBase;
    ThorsAnvil::Nisse::PyntHTTP::Header const& header = headerBase;

    headerBase.add("Content-Length", "56");
    using std::literals::operator""s;
    EXPECT_EQ(1, header.getHeader("content-length"s).size());
    EXPECT_EQ("56", header.getHeader("content-length"s)[0]);

    EXPECT_EQ(0, header.getHeader("Content-Length"s).size());
}

TEST(HeaderTest, AddHeadderConstCharArray)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     headerBase;
    ThorsAnvil::Nisse::PyntHTTP::Header const& header = headerBase;

    headerBase.add("Content-Length", "56");
    EXPECT_EQ(1, header.getHeader("content-length").size());
    EXPECT_EQ("56", header.getHeader("content-length")[0]);

    EXPECT_EQ(0, header.getHeader("Content-Length").size());
}

TEST(HeaderTest, GetInvalidHeader)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;
    EXPECT_EQ(0, header.getHeader("content-length").size());
}

TEST(HeaderTest, ConstGetInvalidHeader)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     headerBase;
    ThorsAnvil::Nisse::PyntHTTP::Header const& header = headerBase;
    EXPECT_EQ(0, header.getHeader("content-length").size());
}

TEST(HeaderTest, DeDupAge)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("age", "14");
    header.add("age", "15");
    EXPECT_EQ(1, header.getHeader("age").size());
    EXPECT_EQ("14", header.getHeader("age")[0]);
}

TEST(HeaderTest, DeDupAuthorization)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("authorization", "14");
    header.add("authorization", "15");
    EXPECT_EQ(1, header.getHeader("authorization").size());
    EXPECT_EQ("14", header.getHeader("authorization")[0]);
}

TEST(HeaderTest, DeDupContentLength)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("content-length", "14");
    header.add("content-length", "15");
    EXPECT_EQ(1, header.getHeader("content-length").size());
    EXPECT_EQ("14", header.getHeader("content-length")[0]);
}

TEST(HeaderTest, DeDupContentType)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("content-type", "14");
    header.add("content-type", "15");
    EXPECT_EQ(1, header.getHeader("content-type").size());
    EXPECT_EQ("14", header.getHeader("content-type")[0]);
}

TEST(HeaderTest, DeDupETag)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("etag", "14");
    header.add("etag", "15");
    EXPECT_EQ(1, header.getHeader("etag").size());
    EXPECT_EQ("14", header.getHeader("etag")[0]);
}

TEST(HeaderTest, DeDupExpires)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("expires", "14");
    header.add("expires", "15");
    EXPECT_EQ(1, header.getHeader("expires").size());
    EXPECT_EQ("14", header.getHeader("expires")[0]);
}

TEST(HeaderTest, DeDupFrom)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("from", "14");
    header.add("from", "15");
    EXPECT_EQ(1, header.getHeader("from").size());
    EXPECT_EQ("14", header.getHeader("from")[0]);
}

TEST(HeaderTest, DeDupHost)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("host", "14");
    header.add("host", "15");
    EXPECT_EQ(1, header.getHeader("host").size());
    EXPECT_EQ("14", header.getHeader("host")[0]);
}

TEST(HeaderTest, DeDupIfModifiedSince)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("if-modified-since", "14");
    header.add("if-modified-since", "15");
    EXPECT_EQ(1, header.getHeader("if-modified-since").size());
    EXPECT_EQ("14", header.getHeader("if-modified-since")[0]);
}

TEST(HeaderTest, DeDupifUnmodifiedSince)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("if-unmodified-since", "14");
    header.add("if-unmodified-since", "15");
    EXPECT_EQ(1, header.getHeader("if-unmodified-since").size());
    EXPECT_EQ("14", header.getHeader("if-unmodified-since")[0]);
}

TEST(HeaderTest, DeDupLastModified)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("last-modified", "14");
    header.add("last-modified", "15");
    EXPECT_EQ(1, header.getHeader("last-modified").size());
    EXPECT_EQ("14", header.getHeader("last-modified")[0]);
}

TEST(HeaderTest, DeDupLocation)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("location", "14");
    header.add("location", "15");
    EXPECT_EQ(1, header.getHeader("location").size());
    EXPECT_EQ("14", header.getHeader("location")[0]);
}

TEST(HeaderTest, DeDupMaxForwards)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("max-forwards", "14");
    header.add("max-forwards", "15");
    EXPECT_EQ(1, header.getHeader("max-forwards").size());
    EXPECT_EQ("14", header.getHeader("max-forwards")[0]);
}

TEST(HeaderTest, DeDupProxyAuthorization)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("proxy-authorization", "14");
    header.add("proxy-authorization", "15");
    EXPECT_EQ(1, header.getHeader("proxy-authorization").size());
    EXPECT_EQ("14", header.getHeader("proxy-authorization")[0]);
}

TEST(HeaderTest, DeDupReferer)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("referer", "14");
    header.add("referer", "15");
    EXPECT_EQ(1, header.getHeader("referer").size());
    EXPECT_EQ("14", header.getHeader("referer")[0]);
}

TEST(HeaderTest, DeDupRetryAfter)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("retry-after", "14");
    header.add("retry-after", "15");
    EXPECT_EQ(1, header.getHeader("retry-after").size());
    EXPECT_EQ("14", header.getHeader("retry-after")[0]);
}

TEST(HeaderTest, DeDupServer)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("server", "14");
    header.add("server", "15");
    EXPECT_EQ(1, header.getHeader("server").size());
    EXPECT_EQ("14", header.getHeader("server")[0]);
}

TEST(HeaderTest, DeDupUserAgent)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("user-agent", "14");
    header.add("user-agent", "15");
    EXPECT_EQ(1, header.getHeader("user-agent").size());
    EXPECT_EQ("14", header.getHeader("user-agent")[0]);
}

TEST(HeaderTest, Iterate)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("head1", "14");
    header.add("head1", "15");
    header.add("head1", "16");
    header.add("head2", "Plop");
    header.add("head2", "Plip");
    header.add("head2", "Poop");
    header.add("head2", "Poop");

    std::size_t                 count = 0;
    std::vector<std::size_t>    size;
    for (auto const& loop: header)
    {
        ++count;
        size.emplace_back(loop.second.size());
    }
    EXPECT_EQ(2, count);
    std::vector<std::size_t> expectedSize{3,4};
    EXPECT_EQ(expectedSize, size);
}

TEST(HeaderTest, SplitOnComma)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("head1", "1, 2,3,  4");
    EXPECT_EQ(4, header.getHeader("head1").size());
    EXPECT_EQ("1", header.getHeader("head1")[0]);
    EXPECT_EQ("2", header.getHeader("head1")[1]);
    EXPECT_EQ("3", header.getHeader("head1")[2]);
    EXPECT_EQ("4", header.getHeader("head1")[3]);
}

TEST(HeaderTest, SplitOnCommaMultipleLines)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("head1", "1, 2,3,  4");
    header.add("head1", "ace,  plan");
    EXPECT_EQ(6, header.getHeader("head1").size());
    EXPECT_EQ("1", header.getHeader("head1")[0]);
    EXPECT_EQ("2", header.getHeader("head1")[1]);
    EXPECT_EQ("3", header.getHeader("head1")[2]);
    EXPECT_EQ("4", header.getHeader("head1")[3]);
    EXPECT_EQ("ace", header.getHeader("head1")[4]);
    EXPECT_EQ("plan", header.getHeader("head1")[5]);
}

TEST(HeaderTest, NoSplitValue)
{
    ThorsAnvil::Nisse::PyntHTTP::Header     header;

    header.add("accept-datetime", "Stuff, Comma No Split, done");
    EXPECT_EQ(1, header.getHeader("accept-datetime").size());
    EXPECT_EQ("Stuff, Comma No Split, done", header.getHeader("accept-datetime")[0]);
}

