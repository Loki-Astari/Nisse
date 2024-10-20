#include <gtest/gtest.h>
#include "StreamOutput.h"
#include "Util.h"
#include <string>

using namespace ThorsAnvil::Nisse::HTTP;

TEST(StreamOutputTest, ConstructBuf)
{
    StreamBufOutput     streamOutput;
}
TEST(StreamOutputTest, ConstrucBuftLength)
{
    std::stringstream   ss;
    StreamBufOutput     streamOutput(ss, 12);
}
TEST(StreamOutputTest, ConstructBufChunked)
{
    std::stringstream   ss;
    StreamBufOutput     streamOutput(ss, Encoding::Chunked);
}
TEST(StreamOutputTest, ConstructBufMove)
{
    std::stringstream   ss;
    StreamBufOutput     streamOutput(ss, Encoding::Chunked);
    StreamBufOutput     move(std::move(streamOutput));
}
TEST(StreamOutputTest, ConstructBufMoveAssign)
{
    std::stringstream   ss;
    StreamBufOutput     streamOutput(ss, Encoding::Chunked);
    StreamBufOutput     move;

    move = std::move(streamOutput);
}

TEST(StreamOutputTest, Construct)
{
    StreamOutput        streamOutput;
    EXPECT_FALSE(static_cast<bool>(streamOutput));
}
TEST(StreamOutputTest, ConstructLength)
{
    std::stringstream   ss;
    StreamOutput        streamOutput(ss, 12);
    EXPECT_TRUE(static_cast<bool>(streamOutput));
}
TEST(StreamOutputTest, ConstructChunked)
{
    std::stringstream   ss;
    StreamOutput        streamOutput(ss, Encoding::Chunked);
    EXPECT_TRUE(static_cast<bool>(streamOutput));
}
TEST(StreamOutputTest, ConstructAssignedLength)
{
    std::stringstream   ss;
    StreamOutput        streamOutput;
    EXPECT_FALSE(static_cast<bool>(streamOutput));

    streamOutput.addBuffer(StreamBufOutput(ss, 12));
    EXPECT_TRUE(static_cast<bool>(streamOutput));
}
TEST(StreamOutputTest, ConstructAssignedChunked)
{
    std::cerr << "ConstructAssignedChunked:: TEST 1\n";
    std::stringstream   ss;
    StreamOutput        streamOutput;
    std::cerr << "ConstructAssignedChunked:: TEST 2\n";
    EXPECT_FALSE(static_cast<bool>(streamOutput));
    std::cerr << "ConstructAssignedChunked:: TEST 3\n";

    std::cerr << "ConstructAssignedChunked:: TEST 4\n";
    streamOutput.addBuffer(StreamBufOutput(ss, Encoding::Chunked));
    std::cerr << "ConstructAssignedChunked:: TEST 5\n";
    EXPECT_TRUE(static_cast<bool>(streamOutput));
    std::cerr << "ConstructAssignedChunked:: TEST DONE\n";
}
TEST(StreamOutputTest, WriteLengthStream)
{
    std::string         data = "This is sime test\n"
                               "One more line XXX\n"
                               "Should not be able to read this";
    std::stringstream   ss;
    StreamOutput        streamOutput(ss, 36);
    streamOutput << data;

    EXPECT_EQ(data.substr(0, 36), ss.str());
}
TEST(StreamOutputTest, WriteChunkedStream)
{
    std::string         data = "This is sime test\n"
                               "One more line XXX\n";
    std::stringstream   ss;
    {
        StreamOutput        streamOutput(ss, Encoding::Chunked);
        streamOutput << data;
    }

    EXPECT_EQ("24\r\nThis is sime test\nOne more line XXX\n\r\n0\r\n", ss.str());
}

TEST(StreamOutputTest, WriteChunkedStreamWithFlush)
{
    std::string         data1 = "This is sime test\n";
    std::string         data2 = "One more line XXX\n";
    std::stringstream   ss;
    {
        StreamOutput        streamOutput(ss, Encoding::Chunked);
        streamOutput << data1 << std::flush << data2;
    }

    EXPECT_EQ("12\r\nThis is sime test\n\r\n12\r\nOne more line XXX\n\r\n0\r\n", ss.str());
}

TEST(StreamOutputTest, WriteChunkedStreamWithFlushFlush)
{
    std::string         data1 = "This is sime test\n";
    std::string         data2 = "One more line XXX\n";
    std::stringstream   ss;
    {
        StreamOutput        streamOutput(ss, Encoding::Chunked);
        streamOutput << data1 << std::flush << data2 << std::flush;
    }

    EXPECT_EQ("12\r\nThis is sime test\n\r\n12\r\nOne more line XXX\n\r\n0\r\n", ss.str());
}

TEST(StreamOutputTest, WriteChunkedWithBufferFill)
{
    std::string         block100 = "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
    std::string         block1000= block100 + block100 + block100 + block100 + block100
                                 + block100 + block100 + block100 + block100 + block100;
    std::string         data1 = "This is sime test\n";
    std::string         data2 = "One more line XXX\n";
    std::stringstream   ss;
    {
        StreamOutput        streamOutput(ss, Encoding::Chunked);
        streamOutput << block1000 << data1 << data2;
    }

    using std::literals::string_literals::operator""s;
    EXPECT_EQ("40C\r\n"s + block1000 + data1 + data2 + "\r\n0\r\n", ss.str());
}

TEST(StreamOutputTest, LengthWithPut)
{
    std::stringstream   ss;
    {
        StreamOutput        streamOutput(ss, 5);
        streamOutput.put('a').put('b').put('c').put('d').put('e');
    }

    EXPECT_EQ("abcde", ss.str());
}

TEST(StreamOutputTest, LengthWithPutWithMore)
{
    std::stringstream   ss;
    {
        StreamOutput        streamOutput(ss, 5);
        streamOutput.put('a').put('b').put('c').put('d').put('e').put('f');
    }

    EXPECT_EQ("abcde", ss.str());
}

TEST(StreamOutputTest, ChunkedWithPut)
{
    std::stringstream   ss;
    {
        StreamOutput        streamOutput(ss, Encoding::Chunked);
        streamOutput.put('a').put('b').put('c').put('d').put('e');
    }

    EXPECT_EQ("5\r\nabcde\r\n0\r\n", ss.str());
}

TEST(StreamOutputTest, ChunkedWithPutWithMore)
{
    std::stringstream   ss;
    {
        StreamOutput        streamOutput(ss, Encoding::Chunked);
        streamOutput.put('a').put('b').put('c').put('d').put('e');
        streamOutput << 15;
    }

    EXPECT_EQ("7\r\nabcde15\r\n0\r\n", ss.str());
}

