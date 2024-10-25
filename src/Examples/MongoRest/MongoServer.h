#ifndef THORSANVIL_NISSE_EXAMPLES_MONGO_SERVER_H
#define THORSANVIL_NISSE_EXAMPLES_MONGO_SERVER_H

#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"
#include "ThorsMongo/ThorsMongo.h"

namespace NHTTP     = ThorsAnvil::Nisse::HTTP;
namespace TAMongo   = ThorsAnvil::DB::Mongo;

namespace ThorsAnvil::Nisse::Examples::MongoRest
{

class MongoServer
{
    TAMongo::ThorsMongo          mongo;
    public:
        MongoServer(std::string const& host, int port, std::string const& user, std::string const& password, std::string const& db);
        // CRUD
        void personCreate(NHTTP::Request& request, NHTTP::Response& response);
        void personGet(NHTTP::Request& request, NHTTP::Response& response);
        void personUpdate(NHTTP::Request& request, NHTTP::Response& response);
        void personDelete(NHTTP::Request& request, NHTTP::Response& response);

        // FIND
        void personFind(NHTTP::Request& request, NHTTP::Response& response);
    private:
        TAMongo::ObjectID   getIdFromRequest(NHTTP::Request& request);
        void                requestFailed(NHTTP::Response& response, std::initializer_list<std::string> messages);
};

}

#endif
