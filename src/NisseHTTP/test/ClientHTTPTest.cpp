#include <gtest/gtest.h>
#include <sys/_types/_mach_port_t.h>
#include <string>
#include <thread>

#include "Request.h"
#include "Response.h"
#include "HTTPHandler.h"
#include "ClientHTTP.h"
#include "NisseHTTPServer.h"

#include "NisseServer/Server.h"

#include "ThorSerialize/PrinterConfig.h"
#include "Util.h"

using namespace ThorsAnvil::Nisse;

class ServerRunner: public HTTP::NisseHTTPServer
{
    public:
        ServerRunner()
            : NisseHTTPServer{1, ThorsAnvil::ThorsSocket::ServerInfo{8080}, ThorsAnvil::ThorsSocket::ServerInfo{8070}}
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
        }
};

class LocalServer
{
    std::thread     thread;
    public:
        LocalServer()
            : thread{[]()
              {
                    ServerRunner                server;
                    server.run();
              }}
        {}
        ~LocalServer()
        {
            HTTP::ClientHTTP    client({"127.0.0.1", 8070}, HTTP::Version::HTTP1_0);
            client.get<std::string>({.path = "/?command=stophard"});
            thread.join();
        }
};

ThorsAnvil::Serialize::PrinterConfig    outputConfig{ThorsAnvil::Serialize::OutputType::Stream};

TEST(ClientHTTPTest, GetChunked)
{
    LocalServer                 server;
    HTTP::ClientHTTP            client({"127.0.0.1", 8080}, HTTP::Version::HTTP1_0);

    std::string                 data = client.get<std::string>({.path = "/pageChunked"});
    EXPECT_EQ("A page with chunked data\nOver 2 lines\n", data);
}

TEST(ClientHTTPTest, GetChunkedWithFlush)
{
    LocalServer                 runner;
    HTTP::ClientHTTP            client({"127.0.0.1", 8080}, HTTP::Version::HTTP1_0);

    std::string                 data = client.get<std::string>({.path = "/pageChunkedWithFlush"});
    EXPECT_EQ("A page with chunked data\nOver 2 lines\nAnother piece\n", data);
}
TEST(ClientHTTPTest, GetSized)
{
    LocalServer                 runner;
    HTTP::ClientHTTP            client({"127.0.0.1", 8080}, HTTP::Version::HTTP1_0);

    std::string                 data = client.get<std::string>({.path = "/pageSized"});
    EXPECT_EQ("A page with sized data\nOver 2 lines\n", data);
}

TEST(ClientHTTPTest, GetSizedWithFlush)
{
    LocalServer                 runner;
    HTTP::ClientHTTP            client({"127.0.0.1", 8080}, HTTP::Version::HTTP1_0);

    std::string                 data = client.get<std::string>({.path = "/pageSizedWithFlush"});
    EXPECT_EQ("A page with sized data\nOver 2 lines\nAnother piece\n", data);
}

TEST(ClientHTTPTest, GetTwoPagesConnectionNotClosed)
{
    LocalServer                 runner;
    HTTP::ClientHTTP            client({"127.0.0.1", 8080}, HTTP::Version::HTTP1_1);    // HTTP1.1 does not close connection by default.

    std::string                 page1 = client.get<std::string>({.path = "/pageSized"});
    EXPECT_EQ("A page with sized data\nOver 2 lines\n", page1);

    std::string                 page2 = client.get<std::string>({.path = "/pageChunked"});
    EXPECT_EQ("A page with chunked data\nOver 2 lines\n", page2);
}

TEST(ClientHTTPTest, GetTwoPagesConnection_IS_Closed)
{
    LocalServer                 runner;
    HTTP::ClientHTTP            client({"127.0.0.1", 8080}, HTTP::Version::HTTP1_0);

    std::string                 page1 = client.get<std::string>({.path = "/pageSized"});
    EXPECT_EQ("A page with sized data\nOver 2 lines\n", page1);

    std::string                 page2 = client.get<std::string>({.path = "/pageChunked"});
    EXPECT_EQ("A page with chunked data\nOver 2 lines\n", page2);
}
