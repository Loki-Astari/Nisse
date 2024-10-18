#include "NisseServer/NisseServer.h"
#include "NisseServer/PyntControl.h"
#include "PyntHTTP/PyntHTTPV1.h"

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

    // Set up the server
    ThorsAnvil::Nisse::NisseServer  server;

    // Processes HTTP connection on port.
    ThorsAnvil::Nisse::PyntHTTP::PyntHTTPV1 http;
    server.listen(initPortSSL, http);
    server.listen(initPort, http);

    ThorsAnvil::Nisse::PyntControl  control(server);
    server.listen(TAS::ServerInfo{port+2}, control);
    server.run();
}
