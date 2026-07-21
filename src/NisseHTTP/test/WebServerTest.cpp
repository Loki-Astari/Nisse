#include "gtest/gtest.h"

#include "Server.h"
#include "ClientHttp.h"


using WebServerRunner = ThorsAnvil::Nisse::Server::UnitTest::ServerRunner<ThorsAnvil::Nisse::HTTP::UnitTest::WebServer>;

TEST(MugServerTest, ServiceRunAddServerWithFileValidateWorks)
{
#ifdef  __WINNT__
    GTEST_SKIP();
#endif

    WebServerRunner     server;

    ThorsAnvil::Nisse::HTTP::ClientHTTP         client({"localhost", 8070});
    ThorsAnvil::Nisse::HTTP::HeaderRequest      headers;
    headers.add("accept", "application/json");
    std::string result = client.get<std::string>({.path = "/files/page1", .headers = headers});

    ASSERT_EQ("Data for page 1\n", result);
}
