#include "gtest/gtest.h"

#include "HeaderResponse.h"

TEST(HeaderResponseTest, Costruct)
{
    ThorsAnvil::Nisse::HTTP::HeaderResponse     header{};
}

TEST(HeaderResponseTest, AddContentLength)
{
    ThorsAnvil::Nisse::HTTP::HeaderResponse     header;

    header.add("Content-Length", "56");
    std::stringstream ss;
    ss << header;

    EXPECT_EQ("", ss.str());
}

TEST(HeaderResponseTest, AddEncoding)
{
    ThorsAnvil::Nisse::HTTP::HeaderResponse     header;

    header.add("transfer-encoding", "chunked");
    std::stringstream ss;
    ss << header;

    EXPECT_EQ("", ss.str());
}

TEST(HeaderResponseTest, AddEncodingAndLength)
{
    ThorsAnvil::Nisse::HTTP::HeaderResponse     header;

    header.add("Content-Length", "56");
    header.add("transfer-encoding", "chunked");
    std::stringstream ss;
    ss << header;

    EXPECT_EQ("", ss.str());
}

TEST(HeaderResponseTest, AddOtherHeaer)
{
    ThorsAnvil::Nisse::HTTP::HeaderResponse     headerBase;
    ThorsAnvil::Nisse::HTTP::HeaderResponse const& header = headerBase;

    headerBase.add("Suffering", "56");
    std::stringstream ss;
    ss << header;

    EXPECT_EQ("Suffering: 56\r\n", ss.str());
}
