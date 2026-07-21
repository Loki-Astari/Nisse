#include "gtest/gtest.h"
#include <sstream>
#include <string>
#include "ClientHTTP.h"
#include "HeaderResponse.h"

using ThorsAnvil::Nisse::HTTP::ClientRequest;
using ThorsAnvil::Nisse::HTTP::ClientHTTPBase;
using ThorsAnvil::Nisse::HTTP::Version;
using ThorsAnvil::Nisse::HTTP::ClientRequest;
using namespace std::string_literals;

TEST(ClientRequestTest, Construct)
{
    std::stringstream   stream;
    ClientHTTPBase      request(stream, Version::HTTP1_1);
    std::string         result;
    request.put({.path = "/bang/bot"}, "Hi there"s, result);

    EXPECT_EQ("PUT /bang/bot HTTP/1.1\r\n"
              "host: localhost\r\n"
              "content-length: 10\r\n"
              "\r\n"
              "\"Hi there\"", stream.str());

}

TEST(ClientRequestTest, ConstructWithAddedHeaders)
{
    std::stringstream   stream;
    ClientHTTPBase      request(stream, Version::HTTP1_1);
    std::string         result;

    ThorsAnvil::Nisse::HTTP::HeaderRequest   headers;
    headers.add("X-Bob", "Meet Here");
    request.put(ClientRequest{.path = "/bang/bot", .headers = headers}, "Hi there"s, result);

    EXPECT_EQ("PUT /bang/bot HTTP/1.1\r\n"
              "host: localhost\r\n"
              "content-length: 10\r\n"
              "x-bob: Meet Here\r\n"
              "\r\n"
              "\"Hi there\"", stream.str());
}
