#include "Server.h"
#include "PintHTTP.h"

#include <ThorsLogging/ThorsLogging.h>
#include <string>
#include <iostream>

int main(int argc, char* argv[])
{
    // ThorsLogging: Log level => 9 all messages (exceptions) displayed: Default std::cerr
    loguru::g_stderr_verbosity = 9;

    std::cout << "Nissa Server\n";

    int             port = 8080;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

#define CERTIFICATE_INFO    "/etc/letsencrypt/live/thorsanvil.dev/"

#ifdef CERTIFICATE_INFO
    // If you have site certificate set CERTIFICATE_INFO to the path
    // This will then create the server to create SSL connections (i.e. HTTPS)
    using ThorsAnvil::ThorsSocket::CertificateInfo;
    using ThorsAnvil::ThorsSocket::SSLctx;
    using ThorsAnvil::ThorsSocket::SServerInfo;
    using ThorsAnvil::ThorsSocket::SSLMethodType;

    CertificateInfo certificate{CERTIFICATE_INFO "fullchain.pem",
                                CERTIFICATE_INFO "privkey.pem"
                               };
    SSLctx          ctx{SSLMethodType::Server, certificate};
    SServerInfo     initPort{port, ctx};

#else
    // Without a site certificate you should only use an normal socket (i.e. HTTP)
    // Note: Most modern browsers are going to complain if you use HTTP.
    //       Though if you are using `curl` or other tools it will work fine.
    //
    // See: https://letsencrypt.org/getting-started/
    //      On how to get a free signed site certificate.
    using ThorsAnvil::ThorsSocket::ServerInfo;
    ServerInfo      initPort{port};
#endif

    // Set up the server
    using ThorsAnvil::Nissa::Server;
    Server          server;

    // Processes HTTP connection on port.
    using ThorsAnvil::Nissa::PintHTTP;
    PintHTTP        http;
    server.listen(initPort, http);
    server.run();
}
