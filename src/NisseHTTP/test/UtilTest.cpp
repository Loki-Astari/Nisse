#include "gtest/gtest.h"
#include "Util.h"
#include <sstream>

using namespace ThorsAnvil::Nisse::HTTP;

TEST(UtilTest, VersionToStream)
{
    std::stringstream   ss;
    ss << Version::HTTP2;

    EXPECT_EQ("HTTP/2", ss.str());
}

TEST(UtilTest, StreamToVersion)
{
    std::stringstream   ss("HTTP/3");
    Version             v = Version::HTTP1_0;
    ss >> v;

    EXPECT_EQ(Version::HTTP3, v);
}

TEST(UtilTest, EncodingToStream)
{
    std::stringstream   ss;
    ss << Encoding::Chunked;

    EXPECT_EQ("chunked", ss.str());
}

TEST(UtilTest, BodyEncodingToStreamContentLength)
{
    std::stringstream   ss;
    ss << BodyEncoding{57};

    EXPECT_EQ("content-length: 57\r\n", ss.str());
}

TEST(UtilTest, BodyEncodingToStreamTransferEncoding)
{
    std::stringstream   ss;
    ss << BodyEncoding{Encoding::Chunked};

    EXPECT_EQ("transfer-encoding: chunked\r\n", ss.str());
}
