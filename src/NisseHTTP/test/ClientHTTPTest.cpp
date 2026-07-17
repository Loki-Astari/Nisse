#include <gtest/gtest.h>
#include <string>

#include "Request.h"
#include "Response.h"
#include "HTTPHandler.h"
#include "ClientHTTP.h"

#include "NisseServer/NisseServer.h"

#include "ThorSerialize/PrinterConfig.h"
#include "Util.h"

using namespace ThorsAnvil::Nisse;

class ServerRunner
{
    Server::NisseServer         server;
    HTTP::HTTPHandler           control;
    std::thread                 thread;

    public:
        ServerRunner()
        {
            server.listen(ThorsAnvil::ThorsSocket::ServerInfo{80}, control);
            control.addPath(HTTP::Method::GET, "/pageGood", [](HTTP::Request const& request, HTTP::Response& response)
            {
                std::cerr << "Page START\n";
                response.body(HTTP::Encoding::Chunked) << "A Good page";
                std::cerr << "Page DONE\n";
                return true;    // Indicates we handeled the request. Don't search for more matches.
            });

            thread  = std::thread([&](){std::cerr << "Server Start\n";server.run();std::cerr << "Server Stop\n";});
            sleep(1);
        }
        ~ServerRunner()
        {
            server.stopHard();
            thread.join();
        }

};

ThorsAnvil::Serialize::PrinterConfig    outputConfig{ThorsAnvil::Serialize::OutputType::Stream};

TEST(ClientHTTPTest, X)
{
    ServerRunner                runner;
    HTTP::ClientHTTP            client({"127.0.0.1", 80});

    std::cerr << "Get\n";
    HTTP::ClientHTTPResponse    input = client.get("/pageGood", HTTP::Version::HTTP1_0);
    std::cerr << "Get DONE\n";

    std::string     line;

    std::cerr << "START\n";
    while (std::getline(input.body(), line)) {
        std::cerr << "\tL >" << line << "<\n";
    }
    std::cerr << "DONE\n";

}
