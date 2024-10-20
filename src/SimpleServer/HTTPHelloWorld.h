#ifndef THORSANVIL_NISSE_SIMPLE_SERVER_HTTP_HELLO_WORLD_H
#define THORSANVIL_NISSE_SIMPLE_SERVER_HTTP_HELLO_WORLD_H

#include "HTTP/PyntHTTP.h"

class HTTPHelloWorld: public ThorsAnvil::Nisse::HTTP::PyntHTTP
{
    public:
        virtual void       processRequest(ThorsAnvil::Nisse::HTTP::Request const& request, ThorsAnvil::Nisse::HTTP::Response& response) override;
};


#endif
