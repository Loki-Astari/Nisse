#include "gtest/gtest.h"
#include <sstream>
#include "ClientResponse.h"

using ThorsAnvil::Nisse::HTTP::ClientResponse;

TEST(ClientResponseTest, Construct)
{
    std::stringstream   stream("HTTP/1.1 200 OK\r\n"
                               "stuff: 8\r\n"
                               "\r\n");
    ClientResponse      response(stream);

    std::stringstream   output;
    output << response;

    EXPECT_EQ("HTTP/1.1 200 OK\r\n"
              "stuff: 8\r\n"
              "\r\n",
              output.str());

}
