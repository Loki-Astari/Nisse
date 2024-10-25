#include "gtest/gtest.h"
#include "HeaderPassThrough.h"
#include <sstream>

using namespace ThorsAnvil::Nisse::HTTP;

TEST(HeaderPassThroughTest, Construct)
{
    std::stringstream   ss;
    HeaderPassThrough   headers(ss);
}

TEST(HeaderPassThroughTest, StreamEmpty)
{
    std::stringstream   ss{""};
    HeaderPassThrough   headers(ss);

    std::stringstream   output;
    output << headers;

    EXPECT_EQ("", output.str());
    EXPECT_EQ(0, std::get<std::streamsize>(headers.getEncoding()));
}

TEST(HeaderPassThroughTest, StreamEmptyHeaderBlock)
{
    std::stringstream   ss{"\r\nMoreDataShouldBeIgnored"};
    HeaderPassThrough   headers(ss);

    std::stringstream   output;
    output << headers;

    EXPECT_EQ("", output.str());
    EXPECT_EQ(0, std::get<std::streamsize>(headers.getEncoding()));
}

TEST(HeaderPassThroughTest, StreamEmptySomeHeader)
{
    std::stringstream   ss{"Header1: data\r\n"
                           "Header2: more data\r\n"
                           "\r\n"
                           "Extra Data"
                          };
    HeaderPassThrough   headers(ss);

    std::stringstream   output;
    output << headers;

    EXPECT_EQ("Header1: data\r\n"
              "Header2: more data\r\n",
              output.str());
    EXPECT_EQ(0, std::get<std::streamsize>(headers.getEncoding()));
}

TEST(HeaderPassThroughTest, StreamEmptySomeHeaderWithContent)
{
    std::stringstream   ss{"Header1: data\r\n"
                           "Content-Length: 56\r\n"
                           "Header2: more data\r\n"
                           "\r\n"
                           "Extra Data"
                          };
    HeaderPassThrough   headers(ss);

    std::stringstream   output;
    output << headers;

    EXPECT_EQ("Header1: data\r\n"
              "Content-Length: 56\r\n"
              "Header2: more data\r\n",
              output.str());
    EXPECT_EQ(56, std::get<std::streamsize>(headers.getEncoding()));
}

TEST(HeaderPassThroughTest, StreamEmptySomeHeaderWithTransferEncoding)
{
    std::stringstream   ss{"Header1: data\r\n"
                           "transfer-EncoDing: chunked\r\n"
                           "Header2: more data\r\n"
                           "\r\n"
                           "Extra Data"
                          };
    HeaderPassThrough   headers(ss);

    std::stringstream   output;
    output << headers;

    EXPECT_EQ("Header1: data\r\n"
              "transfer-EncoDing: chunked\r\n"
              "Header2: more data\r\n",
              output.str());
    EXPECT_EQ(Encoding::Chunked, std::get<Encoding>(headers.getEncoding()));
}



