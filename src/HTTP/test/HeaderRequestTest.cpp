#include "gtest/gtest.h"

#include "HeaderRequest.h"

TEST(HeaderRequestTest, Costruct)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header{};
}

TEST(HeaderRequestTest, AddHeadderStringLiteral)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("Content-Length", "56");
    using std::literals::operator""sv;
    EXPECT_EQ(1, header.getHeader("content-length"sv).size());
    EXPECT_EQ("56", header.getHeader("content-length"sv)[0]);

    EXPECT_EQ(0, header.getHeader("Content-Length"sv).size());
}

TEST(HeaderRequestTest, AddHeadderString)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("Content-Length", "56");
    using std::literals::operator""s;
    EXPECT_EQ(1, header.getHeader("content-length"s).size());
    EXPECT_EQ("56", header.getHeader("content-length"s)[0]);

    EXPECT_EQ(0, header.getHeader("Content-Length"s).size());
}

TEST(HeaderRequestTest, AddHeadderCharArray)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("Content-Length", "56");
    EXPECT_EQ(1, header.getHeader("content-length").size());
    EXPECT_EQ("56", header.getHeader("content-length")[0]);

    EXPECT_EQ(0, header.getHeader("Content-Length").size());
}

TEST(HeaderRequestTest, AddHeadderConstStringLiteral)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     headerBase;
    ThorsAnvil::Nisse::HTTP::HeaderRequest const& header = headerBase;

    headerBase.add("Content-Length", "56");
    using std::literals::operator""sv;
    EXPECT_EQ(1, header.getHeader("content-length"sv).size());
    EXPECT_EQ("56", header.getHeader("content-length"sv)[0]);

    EXPECT_EQ(0, header.getHeader("Content-Length"sv).size());
}

TEST(HeaderRequestTest, AddHeadderConstString)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     headerBase;
    ThorsAnvil::Nisse::HTTP::HeaderRequest const& header = headerBase;

    headerBase.add("Content-Length", "56");
    using std::literals::operator""s;
    EXPECT_EQ(1, header.getHeader("content-length"s).size());
    EXPECT_EQ("56", header.getHeader("content-length"s)[0]);

    EXPECT_EQ(0, header.getHeader("Content-Length"s).size());
}

TEST(HeaderRequestTest, AddHeadderConstCharArray)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     headerBase;
    ThorsAnvil::Nisse::HTTP::HeaderRequest const& header = headerBase;

    headerBase.add("Content-Length", "56");
    EXPECT_EQ(1, header.getHeader("content-length").size());
    EXPECT_EQ("56", header.getHeader("content-length")[0]);

    EXPECT_EQ(0, header.getHeader("Content-Length").size());
}

TEST(HeaderRequestTest, GetInvalidHeader)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;
    EXPECT_EQ(0, header.getHeader("content-length").size());
}

TEST(HeaderRequestTest, ConstGetInvalidHeader)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     headerBase;
    ThorsAnvil::Nisse::HTTP::HeaderRequest const& header = headerBase;
    EXPECT_EQ(0, header.getHeader("content-length").size());
}

TEST(HeaderRequestTest, DeDupAge)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("age", "14");
    header.add("age", "15");
    EXPECT_EQ(1, header.getHeader("age").size());
    EXPECT_EQ("14", header.getHeader("age")[0]);
}

TEST(HeaderRequestTest, DeDupAuthorization)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("authorization", "14");
    header.add("authorization", "15");
    EXPECT_EQ(1, header.getHeader("authorization").size());
    EXPECT_EQ("14", header.getHeader("authorization")[0]);
}

TEST(HeaderRequestTest, DeDupContentLength)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("content-length", "14");
    header.add("content-length", "15");
    EXPECT_EQ(1, header.getHeader("content-length").size());
    EXPECT_EQ("14", header.getHeader("content-length")[0]);
}

TEST(HeaderRequestTest, DeDupContentType)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("content-type", "14");
    header.add("content-type", "15");
    EXPECT_EQ(1, header.getHeader("content-type").size());
    EXPECT_EQ("14", header.getHeader("content-type")[0]);
}

TEST(HeaderRequestTest, DeDupETag)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("etag", "14");
    header.add("etag", "15");
    EXPECT_EQ(1, header.getHeader("etag").size());
    EXPECT_EQ("14", header.getHeader("etag")[0]);
}

TEST(HeaderRequestTest, DeDupExpires)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("expires", "14");
    header.add("expires", "15");
    EXPECT_EQ(1, header.getHeader("expires").size());
    EXPECT_EQ("14", header.getHeader("expires")[0]);
}

TEST(HeaderRequestTest, DeDupFrom)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("from", "14");
    header.add("from", "15");
    EXPECT_EQ(1, header.getHeader("from").size());
    EXPECT_EQ("14", header.getHeader("from")[0]);
}

TEST(HeaderRequestTest, DeDupHost)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("host", "14");
    header.add("host", "15");
    EXPECT_EQ(1, header.getHeader("host").size());
    EXPECT_EQ("14", header.getHeader("host")[0]);
}

TEST(HeaderRequestTest, DeDupIfModifiedSince)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("if-modified-since", "14");
    header.add("if-modified-since", "15");
    EXPECT_EQ(1, header.getHeader("if-modified-since").size());
    EXPECT_EQ("14", header.getHeader("if-modified-since")[0]);
}

TEST(HeaderRequestTest, DeDupifUnmodifiedSince)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("if-unmodified-since", "14");
    header.add("if-unmodified-since", "15");
    EXPECT_EQ(1, header.getHeader("if-unmodified-since").size());
    EXPECT_EQ("14", header.getHeader("if-unmodified-since")[0]);
}

TEST(HeaderRequestTest, DeDupLastModified)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("last-modified", "14");
    header.add("last-modified", "15");
    EXPECT_EQ(1, header.getHeader("last-modified").size());
    EXPECT_EQ("14", header.getHeader("last-modified")[0]);
}

TEST(HeaderRequestTest, DeDupLocation)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("location", "14");
    header.add("location", "15");
    EXPECT_EQ(1, header.getHeader("location").size());
    EXPECT_EQ("14", header.getHeader("location")[0]);
}

TEST(HeaderRequestTest, DeDupMaxForwards)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("max-forwards", "14");
    header.add("max-forwards", "15");
    EXPECT_EQ(1, header.getHeader("max-forwards").size());
    EXPECT_EQ("14", header.getHeader("max-forwards")[0]);
}

TEST(HeaderRequestTest, DeDupProxyAuthorization)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("proxy-authorization", "14");
    header.add("proxy-authorization", "15");
    EXPECT_EQ(1, header.getHeader("proxy-authorization").size());
    EXPECT_EQ("14", header.getHeader("proxy-authorization")[0]);
}

TEST(HeaderRequestTest, DeDupReferer)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("referer", "14");
    header.add("referer", "15");
    EXPECT_EQ(1, header.getHeader("referer").size());
    EXPECT_EQ("14", header.getHeader("referer")[0]);
}

TEST(HeaderRequestTest, DeDupRetryAfter)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("retry-after", "14");
    header.add("retry-after", "15");
    EXPECT_EQ(1, header.getHeader("retry-after").size());
    EXPECT_EQ("14", header.getHeader("retry-after")[0]);
}

TEST(HeaderRequestTest, DeDupServer)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("server", "14");
    header.add("server", "15");
    EXPECT_EQ(1, header.getHeader("server").size());
    EXPECT_EQ("14", header.getHeader("server")[0]);
}

TEST(HeaderRequestTest, DeDupUserAgent)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("user-agent", "14");
    header.add("user-agent", "15");
    EXPECT_EQ(1, header.getHeader("user-agent").size());
    EXPECT_EQ("14", header.getHeader("user-agent")[0]);
}

TEST(HeaderRequestTest, Iterate)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

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

TEST(HeaderRequestTest, SplitOnComma)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("head1", "1, 2,3,  4");
    EXPECT_EQ(4, header.getHeader("head1").size());
    EXPECT_EQ("1", header.getHeader("head1")[0]);
    EXPECT_EQ("2", header.getHeader("head1")[1]);
    EXPECT_EQ("3", header.getHeader("head1")[2]);
    EXPECT_EQ("4", header.getHeader("head1")[3]);
}

TEST(HeaderRequestTest, SplitOnCommaMultipleLines)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

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

TEST(HeaderRequestTest, NoSplitValue)
{
    ThorsAnvil::Nisse::HTTP::HeaderRequest     header;

    header.add("accept-datetime", "Stuff, Comma No Split, done");
    EXPECT_EQ(1, header.getHeader("accept-datetime").size());
    EXPECT_EQ("Stuff, Comma No Split, done", header.getHeader("accept-datetime")[0]);
}

