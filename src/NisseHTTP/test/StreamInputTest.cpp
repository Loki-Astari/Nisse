#include <gtest/gtest.h>
#include "StreamInput.h"
#include "Util.h"

using namespace ThorsAnvil::Nisse::HTTP;

TEST(StreamInputTest, ConstructBuf)
{
    StreamBufInput     streamInput;
}
TEST(StreamInputTest, ConstrucBuftLength)
{
    std::stringstream   ss;
    StreamBufInput      streamInput(ss, 12);
}
TEST(StreamInputTest, ConstructBufChunked)
{
    std::stringstream   ss;
    StreamBufInput      streamInput(ss, Encoding::Chunked);
}
TEST(StreamInputTest, ConstructBufMove)
{
    std::stringstream   ss;
    StreamBufInput      streamInput(ss, Encoding::Chunked);
    StreamBufInput      move(std::move(streamInput));
}
TEST(StreamInputTest, ConstructBufMoveAssign)
{
    std::stringstream   ss;
    StreamBufInput      streamInput(ss, Encoding::Chunked);
    StreamBufInput      move;

    move = std::move(streamInput);
}

TEST(StreamInputTest, Construct)
{
    StreamInput     streamInput;
    EXPECT_FALSE(static_cast<bool>(streamInput));
}
TEST(StreamInputTest, ConstructLength)
{
    std::stringstream   ss;
    StreamInput     streamInput(ss, 12);
    EXPECT_TRUE(static_cast<bool>(streamInput));
}
TEST(StreamInputTest, ConstructChunked)
{
    std::stringstream   ss;
    StreamInput     streamInput(ss, Encoding::Chunked);
    EXPECT_TRUE(static_cast<bool>(streamInput));
}
TEST(StreamInputTest, ConstructAssignedLength)
{
    StreamInput     streamInput;
    EXPECT_FALSE(static_cast<bool>(streamInput));

    std::stringstream   ss;
    streamInput.addBuffer(StreamBufInput(ss, 12));
    EXPECT_TRUE(static_cast<bool>(streamInput));
}
TEST(StreamInputTest, ConstructAssignedChunked)
{
    StreamInput     streamInput;
    EXPECT_FALSE(static_cast<bool>(streamInput));

    std::stringstream   ss;
    streamInput.addBuffer(StreamBufInput(ss, Encoding::Chunked));
    EXPECT_TRUE(static_cast<bool>(streamInput));
}
TEST(StreamInputTest, ReadLengthStream)
{
    std::stringstream   stream{"This is sime test\n"
                               "One more line XXX\n"
                               "Should not be able to read this"
                              };
    StreamInput     streamInput(stream, 36);

    std::string line;
    std::string expected[3] = {"This is sime test", "One more line XXX", "Protection"};
    int count = 0;

    while (std::getline(streamInput, line))
    {
        ASSERT_LT(count, 2);
        EXPECT_EQ(line, expected[count]);
        ++count;
    }
    EXPECT_EQ(2, count);
    EXPECT_TRUE(std::ios::eofbit & streamInput.rdstate());
}
TEST(StreamInputTest, ReadChunkedStream)
{
    std::stringstream   stream{"12\r\nThis is sime test\n\r\n"
                               "12\r\nOne more line XXX\n\r\n"
                               "0\r\n"
                               "Should not be able to read this"
                              };
    StreamInput     streamInput(stream, Encoding::Chunked);

    std::string line;
    std::string expected[3] = {"This is sime test", "One more line XXX", "Protection"};
    int count = 0;

    while (std::getline(streamInput, line))
    {
        ASSERT_LT(count, 2);
        EXPECT_EQ(line, expected[count]);
        ++count;
    }
    EXPECT_EQ(2, count);
    EXPECT_TRUE(std::ios::eofbit & streamInput.rdstate());
}
TEST(StreamInputTest, ReadBulkLengthStream)
{
    std::stringstream   stream{"This is sime test\n"
                               "One more line XXX\n"
                               "Should not be able to read this"
                              };
    StreamInput     streamInput(stream, 36);

    char buffer[100] = {0};
    streamInput.read(buffer, 99);
    std::string_view    bufferView{buffer};
    EXPECT_EQ(bufferView, "This is sime test\nOne more line XXX\n");
}
TEST(StreamInputTest, ReadBulkChunkedStream)
{
    std::stringstream   stream{"12\r\nThis is sime test\n\r\n"
                               "12\r\nOne more line XXX\n\r\n"
                               "0\r\n"
                               "Should not be able to read this"
                              };
    StreamInput     streamInput(stream, Encoding::Chunked);

    char buffer[100] = {0};
    streamInput.read(buffer, 99);
    std::string_view    bufferView{buffer};
    EXPECT_EQ(bufferView, "This is sime test\nOne more line XXX\n");
}

TEST(StreamInputTest, BufSeekBegInShouldFail)
{
    std::stringstream   stream{"12\r\nThis is sime test\n\r\n"
                               "12\r\nOne more line XXX\n\r\n"
                               "0\r\n"
                               "Should not be able to read this"
                              };
    StreamInput     streamInput(stream, Encoding::Chunked);

    char buffer[100] = {0};
    streamInput.read(buffer, 5);
    StreamInput::pos_type newPos = streamInput.rdbuf()->pubseekoff(3, std::ios_base::beg, std::ios_base::in);
    EXPECT_EQ(5, newPos);
}

TEST(StreamInputTest, BufSeekBegOutShouldFail)
{
    std::stringstream   stream{"12\r\nThis is sime test\n\r\n"
                               "12\r\nOne more line XXX\n\r\n"
                               "0\r\n"
                               "Should not be able to read this"
                              };
    StreamInput     streamInput(stream, Encoding::Chunked);

    char buffer[100] = {0};
    streamInput.read(buffer, 5);
    StreamInput::pos_type newPos = streamInput.rdbuf()->pubseekoff(3, std::ios_base::beg, std::ios_base::out);
    EXPECT_EQ(0, newPos);
}

TEST(StreamInputTest, BufSeekEndInShouldFail)
{
    std::stringstream   stream{"12\r\nThis is sime test\n\r\n"
                               "12\r\nOne more line XXX\n\r\n"
                               "0\r\n"
                               "Should not be able to read this"
                              };
    StreamInput     streamInput(stream, Encoding::Chunked);

    char buffer[100] = {0};
    streamInput.read(buffer, 5);
    StreamInput::pos_type newPos = streamInput.rdbuf()->pubseekoff(3, std::ios_base::end, std::ios_base::in);
    EXPECT_EQ(5, newPos);
}

TEST(StreamInputTest, BufSeekEndOutShouldFail)
{
    std::stringstream   stream{"12\r\nThis is sime test\n\r\n"
                               "12\r\nOne more line XXX\n\r\n"
                               "0\r\n"
                               "Should not be able to read this"
                              };
    StreamInput     streamInput(stream, Encoding::Chunked);

    char buffer[100] = {0};
    streamInput.read(buffer, 5);
    StreamInput::pos_type newPos = streamInput.rdbuf()->pubseekoff(3, std::ios_base::end, std::ios_base::out);
    EXPECT_EQ(0, newPos);
}

TEST(StreamInputTest, BufSeekCurOutShouldFail)
{
    std::stringstream   stream{"12\r\nThis is sime test\n\r\n"
                               "12\r\nOne more line XXX\n\r\n"
                               "0\r\n"
                               "Should not be able to read this"
                              };
    StreamInput     streamInput(stream, Encoding::Chunked);

    char buffer[100] = {0};
    streamInput.read(buffer, 5);
    StreamInput::pos_type newPos = streamInput.rdbuf()->pubseekoff(3, std::ios_base::cur, std::ios_base::out);
    EXPECT_EQ(0, newPos);
}

// Seek forward inside current chunk
TEST(StreamInputTest, BufSeekCurIn)
{
    std::stringstream   stream{"12\r\nThis is sime test\n\r\n"
                               "12\r\nOne more line XXX\n\r\n"
                               "0\r\n"
                               "Should not be able to read this"
                              };
    StreamInput     streamInput(stream, Encoding::Chunked);
    // Force an underflow.
    streamInput.peek();

    char buffer[100] = {0};
    std::cerr << "Reading 5\n";
    streamInput.read(buffer, 5);
    std::cerr << "Seeking 3\n";
    StreamInput::pos_type newPos = streamInput.rdbuf()->pubseekoff(3, std::ios_base::cur, std::ios_base::in);
    EXPECT_EQ(8, newPos);
}

// Seek forward to next chunk
TEST(StreamInputTest, BufSeekCurInNextChunk)
{
    std::stringstream   stream{"12\r\nThis is sime test\n\r\n"
                               "12\r\nOne more line XXX\n\r\n"
                               "0\r\n"
                               "Should not be able to read this"
                              };
    StreamInput     streamInput(stream, Encoding::Chunked);

    char buffer[100] = {0};
    streamInput.read(buffer, 5);
    StreamInput::pos_type newPos = streamInput.rdbuf()->pubseekoff(18, std::ios_base::cur, std::ios_base::in);
    EXPECT_EQ(23, newPos);
    EXPECT_EQ('o', streamInput.peek());
}

TEST(StreamInputTest, BufSeekCurInPastEnd)
{
    std::stringstream   stream{"12\r\nThis is sime test\n\r\n"
                               "12\r\nOne more line XXX\n\r\n"
                               "0\r\n"
                               "Should not be able to read this"
                              };
    StreamInput     streamInput(stream, Encoding::Chunked);

    char buffer[100] = {0};
    streamInput.read(buffer, 5);
    StreamInput::pos_type newPos = streamInput.rdbuf()->pubseekoff(36, std::ios_base::cur, std::ios_base::in);
    EXPECT_EQ(36, newPos);
}

TEST(StreamInputTest, BufSeekAbsoluteOutShouldFail)
{
    std::stringstream   stream{"12\r\nThis is sime test\n\r\n"
                               "12\r\nOne more line XXX\n\r\n"
                               "0\r\n"
                               "Should not be able to read this"
                              };
    StreamInput     streamInput(stream, Encoding::Chunked);

    char buffer[100] = {0};
    streamInput.read(buffer, 5);
    StreamInput::pos_type newPos = streamInput.rdbuf()->pubseekpos(36, std::ios_base::out);
    EXPECT_EQ(0, newPos);
}

TEST(StreamInputTest, BufSeekAbsoluteIn)
{
    std::stringstream   stream{"12\r\nThis is sime test\n\r\n"
                               "12\r\nOne more line XXX\n\r\n"
                               "0\r\n"
                               "Should not be able to read this"
                              };
    StreamInput     streamInput(stream, Encoding::Chunked);

    char buffer[100] = {0};
    streamInput.read(buffer, 5);
    StreamInput::pos_type newPos = streamInput.rdbuf()->pubseekpos(12, std::ios_base::in);
    EXPECT_EQ(12, newPos);
    EXPECT_EQ(' ', streamInput.peek());
}

TEST(StreamInputTest, BufSeekAbsoluteInBackFromCurrentFail)
{
    std::stringstream   stream{"12\r\nThis is sime test\n\r\n"
                               "12\r\nOne more line XXX\n\r\n"
                               "0\r\n"
                               "Should not be able to read this"
                              };
    StreamInput     streamInput(stream, Encoding::Chunked);

    char buffer[100] = {0};
    streamInput.read(buffer, 5);
    StreamInput::pos_type newPos = streamInput.rdbuf()->pubseekpos(3, std::ios_base::in);
    EXPECT_EQ(5, newPos);
}

