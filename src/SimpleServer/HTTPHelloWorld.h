#ifndef THORSANVIL_NISSE_SIMPLE_SERVER_HTTP_HELLO_WORLD_H
#define THORSANVIL_NISSE_SIMPLE_SERVER_HTTP_HELLO_WORLD_H

#include "NisseHTTP/PyntHTTP.h"

class HTTPHelloWorld: public ThorsAnvil::Nisse::NisseHTTP::PyntHTTP
{
    public:
        virtual void       processRequest(ThorsAnvil::Nisse::NisseHTTP::Request const& request, ThorsAnvil::Nisse::NisseHTTP::Response& response) override;
};


#endif
