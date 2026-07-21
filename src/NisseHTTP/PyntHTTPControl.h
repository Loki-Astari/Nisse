#ifndef THORSANVIL_NISSE_HTTP_PYNT_HTTP_CONTROL_H
#define THORSANVIL_NISSE_HTTP_PYNT_HTTP_CONTROL_H

/*
 * Server Control Line:
 * Version 1:
 */

#include "NisseHTTPConfig.h"
#include "PyntHTTP.h"
#include "NisseServer/Server.h"

namespace ThorsAnvil::Nisse::HTTP
{

class PyntHTTPControl: public PyntHTTP
{
    Server::Server&    server;
    public:
        PyntHTTPControl(Server::Server& server);
        virtual void processRequest(Request& request, Response& response) override;
};

}

#if defined(NISSE_HEADER_ONLY) && NISSE_HEADER_ONLY == 1
#endif

#endif
