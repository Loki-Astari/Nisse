#ifndef THORSANVIL_NISSE_HTTP_PYNT_HTTP_CONTROL_H
#define THORSANVIL_NISSE_HTTP_PYNT_HTTP_CONTROL_H

/*
 * Server Control Line:
 * Version 1:
 */

#include "NisseHTTPConfig.h"
#include "PyntHTTP.h"

namespace ThorsAnvil::Nisse::HTTP
{

class PyntHTTPControl: public PyntHTTP
{
    Server::NisseServer&    server;
    public:
        PyntHTTPControl(Server::NisseServer& server);
        virtual void processRequest(Request& request, Response& response) override;
};

}

#endif
