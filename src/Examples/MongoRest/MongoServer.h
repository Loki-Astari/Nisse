#ifndef THORSANVIL_NISSE_EXAMPLES_MONGO_SERVER_H
#define THORSANVIL_NISSE_EXAMPLES_MONGO_SERVER_H

#include "NisseServer/NisseServer.h"
#include "NisseServer/Context.h"
#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"
#include "ThorsSocket/Socket.h"
#include "ThorsMongo/ThorsMongo.h"
#include <mutex>

namespace NisServer = ThorsAnvil::Nisse::Server;
namespace NisHttp   = ThorsAnvil::Nisse::HTTP;
namespace TASock    = ThorsAnvil::ThorsSocket;
namespace TAMongo   = ThorsAnvil::DB::Mongo;

namespace ThorsAnvil::Nisse::Examples::MongoRest
{

class LeaseConnection;
class MongoConnectionPool
{
    NisServer::NisseServer&             server;
    std::vector<TAMongo::ThorsMongo>    connections;
    std::mutex                          mutex;
    TASock::Socket                      pipe;
    public:
        MongoConnectionPool(NisServer::NisseServer& server, std::size_t poolSize, std::string_view host, int port, std::string_view user, std::string_view password, std::string_view db)
            : server(server)
            , pipe{TASock::PipeInfo{}, TASock::Blocking::No}
        {
            poolSize = std::max(std::size_t(1), poolSize);
            for (std::size_t loop = 0; loop < poolSize; ++loop)
            {
                connections.emplace_back(TAMongo::MongoURL{std::string(host), port}, TAMongo::Auth::UserNamePassword{std::string(user), std::string(password), std::string(db)});
                pipe.putMessageData(&loop, sizeof(loop));
            }

            server.addResourceQueue(pipe.socketId());
        }
        ~MongoConnectionPool()
        {
            server.remResourceQueue(pipe.socketId());
        }

    private:
        friend class LeaseConnection;
        std::size_t getConnection(NisServer::Context& context)
        {
            std::unique_lock<std::mutex>    lock(mutex);
            int                             socketId = pipe.socketId();
            pipe.setReadYield([&lock, &context, socketId]()
            {
                lock.unlock();
                context.getYield()({NisServer::TaskYieldState::RestoreRead, socketId});
                lock.lock();
                return true;
            });

            std::size_t                     nextValue;
            pipe.getMessageData(&nextValue, sizeof(nextValue));
            return nextValue;
        }
        void returnConnection(std::size_t value)
        {
            std::unique_lock<std::mutex>    lock(mutex);
            pipe.putMessageData(&value, sizeof(value));
        }
};

class LeaseConnection
{
    MongoConnectionPool&    pool;
    std::size_t             mongo;

    public:
        LeaseConnection(MongoConnectionPool& pool, NisServer::Context& context)
            : pool(pool)
            , mongo(pool.getConnection(context))
        {}
        ~LeaseConnection()
        {
            pool.returnConnection(mongo);
        }

        TAMongo::ThorsMongo& connection() const
        {
            return pool.connections[mongo];
        }
};

class MongoServer
{
    MongoConnectionPool     mongoPool;
    public:
        MongoServer(NisServer::NisseServer& server, std::size_t poolSize, std::string_view host, int port, std::string_view user, std::string_view password, std::string_view db);
        // CRUD
        void personCreate(NisHttp::Request& request, NisHttp::Response& response);
        void personGet(NisHttp::Request& request, NisHttp::Response& response);
        void personUpdate(NisHttp::Request& request, NisHttp::Response& response);
        void personDelete(NisHttp::Request& request, NisHttp::Response& response);

        // FIND
        void personFindByName(NisHttp::Request& request, NisHttp::Response& response);
        void personFindByTel(NisHttp::Request& request, NisHttp::Response& response);
        void personFindByZip(NisHttp::Request& request, NisHttp::Response& response);
    private:
        TAMongo::ObjectID   getIdFromRequest(NisHttp::Request& request);
        void                requestFailed(NisHttp::Response& response, std::initializer_list<std::string> messages);
};

}

#endif
