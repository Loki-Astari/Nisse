#include "gtest/gtest.h"
#include <sstream>
#include "ClientRequest.h"
#include "HeaderResponse.h"

using ThorsAnvil::Nisse::HTTP::ClientRequest;

TEST(ClientRequestTest, Construct)
{
    std::stringstream   stream;
    ClientRequest       request(stream, "http://localhost/bang/bot");
    std::ostream&       output = request.body(8);

    output << "Hi there";

    EXPECT_EQ("GET /bang/bot HTTP/1.1\r\n"
              "Host: localhost\r\n"
              "content-length: 8\r\n"
              "\r\n"
              "Hi there", stream.str());

}

TEST(ClientRequestTest, ConstructWithAddedHeaders)
{
    std::stringstream   stream;
    ClientRequest       request(stream, "http://localhost/bang/bot");

    ThorsAnvil::Nisse::HTTP::HeaderResponse   headers;
    headers.add("X-Bob", "Meet Here");

    request.addHeaders(headers);

    std::ostream&       output = request.body(8);
    output << "Hi there";

    EXPECT_EQ("GET /bang/bot HTTP/1.1\r\n"
              "Host: localhost\r\n"
              "X-Bob: Meet Here\r\n"
              "content-length: 8\r\n"
              "\r\n"
              "Hi there", stream.str());

}
