#include <gtest/gtest.h>
#include <sys/_types/_mach_port_t.h>
#include <string>
#include <thread>

#include "Request.h"
#include "Response.h"
#include "HTTPHandler.h"
#include "ClientHTTP.h"
#include "Server.h"

#include "NisseServer/Server.h"

#include "ThorSerialize/JsonThor.h"
#include "ThorSerialize/PrinterConfig.h"
#include "ThorSerialize/Traits.h"
#include "ThorSerialize/SerUtil.h"
#include "Util.h"

using namespace ThorsAnvil::Nisse;

class HTTPTestServer: public HTTP::Server
{
    public:
        HTTPTestServer()
            : Server{1, ThorsAnvil::ThorsSocket::ServerInfo{8080}, ThorsAnvil::ThorsSocket::ServerInfo{8070}}
        {

            addPath(HTTP::Method::GET, "/pageChunked", [](HTTP::Request const& request, HTTP::Response& response)
            {
                response.body(HTTP::Encoding::Chunked) << "\"A page with chunked data\\nOver 2 lines\\n\"";
                return true;    // Indicates we handeled the request. Don't search for more matches.
            });
            addPath(HTTP::Method::GET, "/pageChunkedWithFlush", [](HTTP::Request const& request, HTTP::Response& response)
            {
                response.body(HTTP::Encoding::Chunked) << "\"A page with chunked data\\nOver 2 lines\\n" << std::flush
                                                       << "Another piece\\n\"";
                return true;    // Indicates we handeled the request. Don't search for more matches.
            });
            addPath(HTTP::Method::GET, "/pageSized", [](HTTP::Request const& request, HTTP::Response& response)
            {
                response.body(40) << "\"A page with sized data\\nOver 2 lines\\n\"";
                return true;    // Indicates we handeled the request. Don't search for more matches.
            });
            addPath(HTTP::Method::GET, "/pageSizedWithFlush", [](HTTP::Request const& request, HTTP::Response& response)
            {
                response.body(55) << "\"A page with sized data\\nOver 2 lines\\n" << std::flush
                                  << "Another piece\\n\"";
                return true;    // Indicates we handeled the request. Don't search for more matches.
            });

            addPath(HTTP::Method::POST, "/dont/read/post/chunked", [](HTTP::Request const& request, HTTP::Response& response)
            {
                std::string data;
                request.body() >> ThorsAnvil::Serialize::jsonImporter(data);
                std::cerr << "READ: " << data << "\n\n";
                //response.setStatus(403);
                // Note: Explicitly not reading any posted data.
                response.body(HTTP::Encoding::Chunked) << R"("Received 403")";
                return true;
            });
            addPath(HTTP::Method::POST, "/dont/read/post/sized", [](HTTP::Request const& request, HTTP::Response& response)
            {
                response.setStatus(403);
                // Note: Explicitly not reading any posted data.
                response.body(14) << R"("Received 403")";
                return true;
            });
        }
};

using HTTPTestServerRunner = ThorsAnvil::Nisse::Server::UnitTest::ServerRunner<HTTPTestServer>;

ThorsAnvil::Serialize::PrinterConfig    outputConfig{ThorsAnvil::Serialize::OutputType::Stream};

TEST(ClientHTTPTest, GetChunked)
{
    HTTPTestServerRunner        server;
    HTTP::ClientHTTP            client({"127.0.0.1", 8080}, HTTP::Version::HTTP1_0);

    std::string                 data = client.get<std::string>({.path = "/pageChunked"});
    EXPECT_EQ("A page with chunked data\nOver 2 lines\n", data);
}

TEST(ClientHTTPTest, GetChunkedWithFlush)
{
    HTTPTestServerRunner        server;
    HTTP::ClientHTTP            client({"127.0.0.1", 8080}, HTTP::Version::HTTP1_0);

    std::string                 data = client.get<std::string>({.path = "/pageChunkedWithFlush"});
    EXPECT_EQ("A page with chunked data\nOver 2 lines\nAnother piece\n", data);
}
TEST(ClientHTTPTest, GetSized)
{
    HTTPTestServerRunner        server;
    HTTP::ClientHTTP            client({"127.0.0.1", 8080}, HTTP::Version::HTTP1_0);

    std::string                 data = client.get<std::string>({.path = "/pageSized"});
    EXPECT_EQ("A page with sized data\nOver 2 lines\n", data);
}

TEST(ClientHTTPTest, GetSizedWithFlush)
{
    HTTPTestServerRunner        server;
    HTTP::ClientHTTP            client({"127.0.0.1", 8080}, HTTP::Version::HTTP1_0);

    std::string                 data = client.get<std::string>({.path = "/pageSizedWithFlush"});
    EXPECT_EQ("A page with sized data\nOver 2 lines\nAnother piece\n", data);
}

TEST(ClientHTTPTest, GetTwoPagesConnectionNotClosed)
{
    HTTPTestServerRunner        server;
    HTTP::ClientHTTP            client({"127.0.0.1", 8080}, HTTP::Version::HTTP1_1);    // HTTP1.1 does not close connection by default.

    std::string                 page1 = client.get<std::string>({.path = "/pageSized"});
    EXPECT_EQ("A page with sized data\nOver 2 lines\n", page1);

    std::string                 page2 = client.get<std::string>({.path = "/pageChunked"});
    EXPECT_EQ("A page with chunked data\nOver 2 lines\n", page2);
}

TEST(ClientHTTPTest, GetTwoPagesConnection_IS_Closed)
{
    HTTPTestServerRunner        server;
    HTTP::ClientHTTP            client({"127.0.0.1", 8080}, HTTP::Version::HTTP1_0);

    std::string                 page1 = client.get<std::string>({.path = "/pageSized"});
    EXPECT_EQ("A page with sized data\nOver 2 lines\n", page1);

    std::string                 page2 = client.get<std::string>({.path = "/pageChunked"});
    EXPECT_EQ("A page with chunked data\nOver 2 lines\n", page2);
}

TEST(ClientHTTPTest, ServerDoesNotExplicitlyReadBody)
{
    using namespace std::string_literals;
    HTTPTestServerRunner        server;
    HTTP::ClientHTTP            client({"127.0.0.1", 8080}, HTTP::Version::HTTP1_1);

    std::string                 page1 = client.post<std::string>({.path = "/dont/read/post/chunked"}, "This is the input string"s);
    EXPECT_EQ("Received 403", page1);

    std::cerr << "\n\n\n\n\n================\n";

    std::string                 page2 = client.get<std::string>({.path = "/pageChunked"});
    //EXPECT_EQ("A page with chunked data\nOver 2 lines\n", page2);
}

