#include "Nissa.h"
#include "Server.h"
#include "PintHTTP.h"

#include "ThorsLogging/ThorsLogging.h"
#include <string>
#include <iostream>

using namespace ThorsAnvil::Nissa;

int main(int argc, char* argv[])
{
    using ThorsAnvil::ThorsSocket::CertificateInfo;
    std::cerr << PACKAGE_STRING << " Server\n";

    loguru::g_stderr_verbosity = 9;
    CertificateInfo certificate{"/etc/letsencrypt/live/thorsanvil.dev/fullchain.pem",
                                "/etc/letsencrypt/live/thorsanvil.dev/privkey.pem"
                               };
    Server          server{certificate};
    int             port = 8080;

    if (argc > 1) {
        port = std::stoi(argv[1]);
    }
    PintHTTP    http;
    server.listen(port, http);
    server.listen(port + 1, http);
    server.run();
}
