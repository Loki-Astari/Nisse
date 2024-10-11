#include "Server.h"
#include "PintHTTP.h"

#include <ThorsLogging/ThorsLogging.h>
#include <string>
#include <iostream>

int main(int argc, char* argv[])
{
    // ThorsLogging: Log level => 9 all messages (exceptions) displayed: Default std::cerr
    loguru::g_stderr_verbosity = 9;

    std::cerr << PACKAGE_STRING << " Server\n";

    // My Certificate to use an HTTPS connection.
    // Remove if you don't have a certificate. The server will default to HTTP connection.
    using ThorsAnvil::ThorsSocket::CertificateInfo;
    CertificateInfo certificate{"/etc/letsencrypt/live/thorsanvil.dev/fullchain.pem",
                                "/etc/letsencrypt/live/thorsanvil.dev/privkey.pem"
                               };

    using ThorsAnvil::Nissa::Server;
    using ThorsAnvil::Nissa::PintHTTP;

    Server          server{certificate};

    int             port = 8080;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

    // Processes HTTP connection on port.
    PintHTTP    http;
    server.listen(port, http);
    server.run();
}
