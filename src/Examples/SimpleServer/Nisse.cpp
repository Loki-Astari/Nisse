#include "NisseServer/NisseServer.h"
#include "NisseServer/PyntControl.h"
#include "NisseHTTP/HTTPHandler.h"
#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"
#include <ThorsLogging/ThorsLogging.h>
#include <ThorsSocket/Server.h>

namespace TAS       = ThorsAnvil::ThorsSocket;
namespace NServer   = ThorsAnvil::Nisse::Server;
namespace NHTTP     = ThorsAnvil::Nisse::HTTP;

int main(int argc, char* argv[])
{
    // ThorsLogging: Log level => 9 all messages (exceptions) displayed: Default std::cerr
    loguru::g_stderr_verbosity = 9;

    int             port = 8080;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }
    std::cout << "Nisse Server: " << port << "\n";
    // Note: Below we create three accept sockets:
    //      port:       https:// connection
    //      port + 1:   http://  connection
    //      port + 2:   if you connect to this the server will simply stop.


    // Part 1:
    // =======
    // Object to processes HTTP connections with.
    // Register with listen() below.
    NHTTP::HTTPHandler   http;
    http.addPath("/HW{Who}.html", [](NHTTP::Request& request, NHTTP::Response& response)
    {
        std::string who  = request.variables()["Who"];
        std::string data = R"(
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head><title>Nisse server 1.1</title></head>
<body>Hello world: )"  + who + R"(</body>
</html>
)";
        NHTTP::HeaderResponse   header;
        response.addHeaders(header);
        response.body(data.size()) << data;
    });
    http.addPath("/CK{Who}.html", [](NHTTP::Request& request, NHTTP::Response& response)
    {
        NHTTP::HeaderResponse   header;
        response.addHeaders(header);
        response.body(NHTTP::Encoding::Chunked) << R"(
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head><title>Nisse server 1.1</title></head>
<body>Hello world: )" << request.variables()["Who"] << R"(</body>
</html>
)";
    });

    // Part 2:
    // =======
    // Set up the accept port initialization.
#define CERTIFICATE_INFO    "/etc/letsencrypt/live/thorsanvil.dev/"

#ifdef CERTIFICATE_INFO
    // If you have site certificate set CERTIFICATE_INFO to the path
    // This will allows us to set up the SSL information needed for an HTTPS connection.
    //
    // See: https://letsencrypt.org/getting-started/
    //      On how to get a free signed site certificate.
    TAS::CertificateInfo certificate{CERTIFICATE_INFO "fullchain.pem",
                                     CERTIFICATE_INFO "privkey.pem"
                                    };
    TAS::SSLctx         ctx{TAS::SSLMethodType::Server, certificate};
    TAS::SServerInfo    initPortSSL{port, ctx};
#endif
    // Without a site certificate you should only use an normal socket (i.e. HTTP)
    // Note: Most modern browsers are going to complain if you use HTTP.
    //       Though if you are using `curl` or other tools it will work fine.
    TAS::ServerInfo      initPort{port+1};


    // Part 3:
    // =======
    // Set up the server
    NServer::NisseServer              server;

    // Set up the ports we want to listen on
    // and the object that will handle the requests from that port.
#ifdef CERTIFICATE_INFO
    server.listen(initPortSSL, http);
#endif
    server.listen(initPort, http);

    // This interface does nothing.
    // But if you connect to it (port+2) it will cleanly shutdown the server.
    // But you can hit ctrl-c will usually work.
    NServer::PyntControl  control(server);
    server.listen(TAS::ServerInfo{port+2}, control);


    // Part 4:
    // =======
    // Let the server run.
    server.run();
}
