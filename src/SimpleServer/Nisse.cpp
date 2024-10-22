#include "NisseServer/NisseServer.h"
#include "NisseServer/PyntControl.h"
#include "NisseHTTP/HTTPHandler.h"
#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"

#include <ThorsLogging/ThorsLogging.h>
#include <ThorsSocket/Server.h>
#include <string>
#include <iostream>

namespace TAS   = ThorsAnvil::ThorsSocket;

int main(int argc, char* argv[])
{
    // ThorsLogging: Log level => 9 all messages (exceptions) displayed: Default std::cerr
    loguru::g_stderr_verbosity = 9;

    std::cout << "Nisse Server\n";

    int             port = 8080;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

#define CERTIFICATE_INFO    "/etc/letsencrypt/live/thorsanvil.dev/"

#ifdef CERTIFICATE_INFO
    // If you have site certificate set CERTIFICATE_INFO to the path
    // This will then create the server to create SSL connections (i.e. HTTPS)

    TAS::CertificateInfo certificate{CERTIFICATE_INFO "fullchain.pem",
                                     CERTIFICATE_INFO "privkey.pem"
                                    };
    TAS::SSLctx         ctx{TAS::SSLMethodType::Server, certificate};
    TAS::SServerInfo    initPortSSL{port, ctx};

#endif
    // Without a site certificate you should only use an normal socket (i.e. HTTP)
    // Note: Most modern browsers are going to complain if you use HTTP.
    //       Though if you are using `curl` or other tools it will work fine.
    //
    // See: https://letsencrypt.org/getting-started/
    //      On how to get a free signed site certificate.
    TAS::ServerInfo      initPort{port+1};

    // Processes HTTP connection on port.
    ThorsAnvil::Nisse::NisseHTTP::HTTPHandler   http;
    http.addPath("/HW{Who}.html", [](ThorsAnvil::Nisse::NisseHTTP::Request& request, ThorsAnvil::Nisse::NisseHTTP::Response& response)
    {
        std::string who  = request.variables()["Who"];
        std::string data = R"(
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head><title>Nisse server 1.1</title></head>
<body>Hello world: )"  + who + R"(</body>
</html>
)";
        ThorsAnvil::Nisse::NisseHTTP::HeaderResponse   header;
        response.addHeaders(header, data.size()) << data;
    });
    http.addPath("/CK{Who}.html", [](ThorsAnvil::Nisse::NisseHTTP::Request& request, ThorsAnvil::Nisse::NisseHTTP::Response& response)
    {
        ThorsAnvil::Nisse::NisseHTTP::HeaderResponse   header;
        response.addHeaders(header, ThorsAnvil::Nisse::NisseHTTP::Encoding::Chunked) << R"(
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head><title>Nisse server 1.1</title></head>
<body>Hello world: )" << request.variables()["Who"] << R"(</body>
</html>
)";
    });

    // Set up the server
    ThorsAnvil::Nisse::NisseServer              server;

#ifdef CERTIFICATE_INFO
    server.listen(initPortSSL, http);
#endif
    server.listen(initPort, http);

    // This interface does nothing.
    // But if you connect to it (port+2) it will cleanly shutdown the server.
    // But you can hit ctrl-c will usually work.
    ThorsAnvil::Nisse::PyntControl  control(server);
    server.listen(TAS::ServerInfo{port+2}, control);
    server.run();
}
