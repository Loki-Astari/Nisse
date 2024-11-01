#ifndef THORSANVIL_NISSE_EXAMPLES_MONGO_SERVER_H
#define THORSANVIL_NISSE_EXAMPLES_MONGO_SERVER_H

#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"
#include "ThorsMongo/ThorsMongo.h"

namespace NHTTP     = ThorsAnvil::Nisse::HTTP;
namespace TAMongo   = ThorsAnvil::DB::Mongo;

namespace ThorsAnvil::Nisse::Examples::MongoRest
{

class MongoConnectionPool
{
    std::vector<TAMongo::ThorsMongo>    connections;
    std::size_t                         next;
    public:
        MongoConnectionPool(std::size_t poolSize, std::string const& host, int port, std::string const& user, std::string const& password, std::string const& db)
            : next(0)
        {
            poolSize = std::max(std::size_t(1), poolSize);
            for (std::size_t loop = 0; loop < poolSize; ++loop)
            {
                connections.emplace_back(TAMongo::MongoURL{host, port}, TAMongo::Auth::UserNamePassword{user, password, db});
            }
        }

        TAMongo::ThorsMongo&    getConnection()
        {
            // Bad implementation.
            // Will work for low volume server that is only handling upto poolSize request simultaneously.
            return connections[next++ % connections.size()];
        }
};

class MongoServer
{
    MongoConnectionPool     mongoPool;
    public:
        MongoServer(std::size_t poolSize, std::string const& host, int port, std::string const& user, std::string const& password, std::string const& db);
        // CRUD
        void personCreate(NHTTP::Request& request, NHTTP::Response& response);
        void personGet(NHTTP::Request& request, NHTTP::Response& response);
        void personUpdate(NHTTP::Request& request, NHTTP::Response& response);
        void personDelete(NHTTP::Request& request, NHTTP::Response& response);

        // FIND
        void personFindByName(NHTTP::Request& request, NHTTP::Response& response);
        void personFindByTel(NHTTP::Request& request, NHTTP::Response& response);
        void personFindByZip(NHTTP::Request& request, NHTTP::Response& response);
    private:
        TAMongo::ObjectID   getIdFromRequest(NHTTP::Request& request);
        void                requestFailed(NHTTP::Response& response, std::initializer_list<std::string> messages);
};

}

#endif
