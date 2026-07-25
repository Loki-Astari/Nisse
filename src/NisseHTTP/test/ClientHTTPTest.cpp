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
            addPath(HTTP::Method::POST, "/MCPIssue", [](HTTP::Request const& request, HTTP::Response& response)
            {
                response.body(HTTP::Encoding::Chunked) << R"({"jsonrpc":"2.0","id":1,"error":{"code":-32600,"message":"A bad thing happened"}}))";
                return true;
            });
            addPath(HTTP::Method::POST, "/ClientSendsLessThanServerExpects", [](HTTP::Request const& request, HTTP::Response& response)
            {
                std::istream& body = request.body();

                char next;
                body >> next;
                EXPECT_EQ('[', next);

                if (body >> next) {
                    EXPECT_TRUE(false); // We only sent a single character and that was just read.
                }
                else {
                    EXPECT_TRUE(true);
                }

                response.body(HTTP::Encoding::Chunked) << R"({"jsonrpc":"2.0","error":{"code":-32700,"message":"Parse error"},"id":null})";
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


    std::string                 page2 = client.get<std::string>({.path = "/pageChunked"});
    //EXPECT_EQ("A page with chunked data\nOver 2 lines\n", page2);
}

TEST(ClientHTTPTest, MCPIssue)
{
    HTTPTestServerRunner        server;

    ThorsAnvil::Nisse::HTTP::ClientHTTP client{ThorsAnvil::ThorsSocket::SocketInfo{"localhost", 8080}, ThorsAnvil::Nisse::HTTP::Version::HTTP1_0 };
    client.send(ThorsAnvil::Nisse::HTTP::Method::POST, {.path = "/MCPIssue"}, ThorsAnvil::Nisse::HTTP::Encoding::Chunked, [&](ThorsAnvil::Nisse::HTTP::StreamOutput& out)
    {
        out << R"({"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025_11_25"}})";
        return true;
    });
    client.processResp([](ThorsAnvil::Nisse::HTTP::ClientHTTPResponse const& resp)
    {
        using namespace std::string_view_literals;
        EXPECT_EQ(resp.getStatus(), 200);
        EXPECT_EQ(resp.getMessage(), "OK");
        ASSERT_TRUE(resp.getHeader().hasHeader("transfer-encoding"sv));
        EXPECT_EQ(resp.getHeader().getHeader("transfer-encoding")[0], "chunked");

        std::cerr << "Status:  " << resp.getStatus() << "\n"
                  << "Message: " << resp.getMessage() << "\n"
                  << "Version: " << resp.getVersion() << "\n"
                  << "Header:  " << resp.getHeader() << "\n"
                  << "Body:   >" << resp.body().rdbuf() << "<\n";
    });

}

TEST(ClientHTTPTest, ClientSendsLessThanServerExpects)
{
    HTTPTestServerRunner        server;
    std::ostringstream   result;

    ThorsAnvil::Nisse::HTTP::ClientHTTP  client({"localhost", 8080});
    client.send(ThorsAnvil::Nisse::HTTP::Method::POST, {.path = "/ClientSendsLessThanServerExpects"}, ThorsAnvil::Nisse::HTTP::Encoding::Chunked, [&](ThorsAnvil::Nisse::HTTP::StreamOutput& action)
    {
        action << R"([)";
    });
    client.processResp([&](ThorsAnvil::Nisse::HTTP::ClientHTTPResponse const& resp)
    {
        result << resp.body().rdbuf();
    });

    EXPECT_EQ(R"({"jsonrpc":"2.0","error":{"code":-32700,"message":"Parse error"},"id":null})", result.str());
}


