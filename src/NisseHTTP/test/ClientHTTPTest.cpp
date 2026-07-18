#include <gtest/gtest.h>
#include <string>

#include "Request.h"
#include "Response.h"
#include "HTTPHandler.h"
#include "ClientHTTP.h"
#include "NisseHTTPServer.h"

#include "NisseServer/NisseServer.h"

#include "ThorSerialize/PrinterConfig.h"
#include "Util.h"

using namespace ThorsAnvil::Nisse;

class ServerRunner: public HTTP::NisseHTTPServer
{
    public:
        ServerRunner()
            : NisseHTTPServer{1, ThorsAnvil::ThorsSocket::ServerInfo{8079}, ThorsAnvil::ThorsSocket::ServerInfo{8070}}
        {

            addPath(HTTP::Method::GET, "/pageChunked", [](HTTP::Request const& request, HTTP::Response& response)
            {
                response.body(HTTP::Encoding::Chunked) << "A page with chunked data\nOver 2 lines\n";
                return true;    // Indicates we handeled the request. Don't search for more matches.
            });
            addPath(HTTP::Method::GET, "/pageChunkedWithFlush", [](HTTP::Request const& request, HTTP::Response& response)
            {
                response.body(HTTP::Encoding::Chunked) << "A page with chunked data\nOver 2 lines\n" << std::flush
                                                       << "Another piece";
                return true;    // Indicates we handeled the request. Don't search for more matches.
            });
            addPath(HTTP::Method::GET, "/pageSized", [](HTTP::Request const& request, HTTP::Response& response)
            {
                response.body(36) << "A page with sized data\nOver 2 lines\n";
                return true;    // Indicates we handeled the request. Don't search for more matches.
            });
            addPath(HTTP::Method::GET, "/pageSizedWithFlush", [](HTTP::Request const& request, HTTP::Response& response)
            {
                response.body(49) << "A page with sized data\nOver 2 lines\n" << std::flush
                                  << "Another piece";
                return true;    // Indicates we handeled the request. Don't search for more matches.
            });
        }
};

ThorsAnvil::Serialize::PrinterConfig    outputConfig{ThorsAnvil::Serialize::OutputType::Stream};

TEST(ClientHTTPTest, GetChunked)
{
    ServerRunner                runner;
    HTTP::ClientHTTP            client({"127.0.0.1", 8079}, HTTP::Version::HTTP1_0);

    client.get({.path = "/pageChunked"});

    std::string data;
    client.processResp([&data](HTTP::ClientHTTPResponse const& response)
    {
        std::istream&  input = response.body();

        std::string line;
        while (std::getline(input, line)) {
            data.append(line);
            data.append("\n");
        }
    });

    EXPECT_EQ("A page with chunked data\nOver 2 lines\n", data);
}

TEST(ClientHTTPTest, GetChunkedWithFlush)
{
    ServerRunner                runner;
    HTTP::ClientHTTP            client({"127.0.0.1", 8079}, HTTP::Version::HTTP1_0);

    client.get({.path = "/pageChunkedWithFlush"});

    std::string data;
    client.processResp([&data](HTTP::ClientHTTPResponse const& response)
    {
        std::istream&  input = response.body();

        std::string line;
        while (std::getline(input, line)) {
            data.append(line);
            data.append("\n");
        }
    });

    EXPECT_EQ("A page with chunked data\nOver 2 lines\nAnother piece\n", data);
}
TEST(ClientHTTPTest, GetSized)
{
    ServerRunner                runner;
    HTTP::ClientHTTP            client({"127.0.0.1", 8079}, HTTP::Version::HTTP1_0);

    client.get({.path = "/pageSized"});

    std::string data;
    client.processResp([&data](HTTP::ClientHTTPResponse const& response)
    {
        std::istream&  input = response.body();

        std::string line;
        while (std::getline(input, line)) {
            data.append(line);
            data.append("\n");
        }
    });

    EXPECT_EQ("A page with sized data\nOver 2 lines\n", data);
}

TEST(ClientHTTPTest, GetSizedWithFlush)
{
    ServerRunner                runner;
    HTTP::ClientHTTP            client({"127.0.0.1", 8079}, HTTP::Version::HTTP1_0);

    client.get({.path = "/pageSizedWithFlush"});

    std::string data;
    client.processResp([&data](HTTP::ClientHTTPResponse const& response)
    {
        std::istream&  input = response.body();

        std::string line;
        while (std::getline(input, line)) {
            data.append(line);
            data.append("\n");
        }
    });

    EXPECT_EQ("A page with sized data\nOver 2 lines\nAnother piece\n", data);
}
