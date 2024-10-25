#ifndef THORSANVIL_NISSE_EXAMPLES_MONGO_SERVER_H
#define THORSANVIL_NISSE_EXAMPLES_MONGO_SERVER_H

#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"

namespace NHTTP = ThorsAnvil::Nisse::HTTP;

namespace ThorsAnvil::Nisse::Examples::MongoRest
{

class MongoServer
{
    public:
        void personCreate(NHTTP::Request& request, NHTTP::Response& response);
        void personGet(NHTTP::Request& request, NHTTP::Response& response);
        void personUpdate(NHTTP::Request& request, NHTTP::Response& response);
        void personDelete(NHTTP::Request& request, NHTTP::Response& response);
};

}

#endif
