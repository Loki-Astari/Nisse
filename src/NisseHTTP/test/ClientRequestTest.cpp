#include "gtest/gtest.h"
#include <sstream>
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
    ClientHTTPBase      request(stream, "localhost", Version::HTTP1_1);
    request.put({path: "/bang/bot"}, "Hi there"s);

    EXPECT_EQ("PUT /bang/bot HTTP/1.1\r\n"
              "host: localhost\r\n"
              "content-length: 10\r\n"
              "\r\n"
              "\"Hi there\"", stream.str());

}

TEST(ClientRequestTest, ConstructWithAddedHeaders)
{
    std::stringstream   stream;
    ClientHTTPBase      request(stream, "localhost", Version::HTTP1_1);

    ThorsAnvil::Nisse::HTTP::HeaderRequest   headers;
    headers.add("X-Bob", "Meet Here");
    request.put(ClientRequest{path: "/bang/bot", headers: headers}, "Hi there"s);

    EXPECT_EQ("PUT /bang/bot HTTP/1.1\r\n"
              "host: localhost\r\n"
              "content-length: 10\r\n"
              "x-bob: Meet Here\r\n"
              "\r\n"
              "\"Hi there\"", stream.str());
}
