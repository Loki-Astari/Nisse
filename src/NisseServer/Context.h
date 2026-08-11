#ifndef THORSANVIL_NISSE_SERVER_CONTEXT_H
#define THORSANVIL_NISSE_SERVER_CONTEXT_H

#include "NisseServerConfig.h"
#include "NisseUtil.h"
#include "EventHandlerLibEvent.h"
#include <ThorsSocket/Socket.h>
#include <ThorsSocket/SocketStream.h>

#include <thread>

namespace TASock = ThorsAnvil::ThorsSocket;

namespace ThorsAnvil::Nisse::Server
{

class Server;
class ContextThreadNotify;
class ContextThreadNotifyYield;
class ContextThreadNotifyInterface;

class Context
{
    Server&         server;
    Yield&          yield;
    int             owner;

    friend class ContextThreadNotify;
    friend class ContextThreadNotifyYield;
    ContextThreadNotifyInterface*  notify;
    public:
        Context(Server& server, Yield& yield, int owner);
        void registerOwnedSocketStream(TASock::SocketStream& stream, EventType initialWait);
        void unregisterOwnedSocketStream(TASock::SocketStream& stream);
        void registerOwnedSocket(TASock::Socket& socket, EventType initialWait);
        void unregisterOwnedSocket(TASock::Socket& socket);
        bool isFeatureEnabled(Feature feature) const;
        static void registerSharedSocket(Server& server, TASock::Socket& socket);
        static void unregisterSharedSocket(Server& server, TASock::Socket& socket);

        Yield&      getYield()  {return yield;}
    private:
        void registerYield(int fd, EventType initialWait, TASock::Socket& socket, TASock::YieldFunc&& readYield, TASock::YieldFunc&& writeYield);
        void unregisterYield(int fd);
};

// The RAII classes to correctly utilize the Context object.
class AsyncStream
{
    TASock::SocketStream&   stream;
    Context&                context;
    public:
        AsyncStream(TASock::SocketStream& stream, Context& context, EventType initialWait);
        ~AsyncStream();
};

class AsyncSocket
{
    TASock::Socket&         socket;
    Context&                context;
    public:
        AsyncSocket(TASock::Socket& socket, Context& context, EventType initialWait);
        ~AsyncSocket();
};

class AsyncSharedSocket
{
    TASock::Socket&         socket;
    Server&                 server;
    public:
        AsyncSharedSocket(TASock::Socket& socket, Server& server);
        ~AsyncSharedSocket();
};

class ContextThreadNotifyInterface
{
    public:
        virtual ~ContextThreadNotifyInterface() {}
        virtual void notifyThreadJoinContext(std::thread::id id)   = 0;
        virtual void notifyThreadYieldContext(std::thread::id id)  = 0;
};

class ContextThreadNotify
{
    Context&                        context;
    public:
        ContextThreadNotify(Context& context, ContextThreadNotifyInterface& notifier)
            : context(context)
        {
            context.notify = &notifier;
            context.notify->notifyThreadJoinContext(std::this_thread::get_id());
        }
        ~ContextThreadNotify()
        {
            context.notify->notifyThreadYieldContext(std::this_thread::get_id());
            context.notify = nullptr;
        }
};
class ContextThreadNotifyYield
{
    Context&                        context;
    public:
        ContextThreadNotifyYield(Context& context)
            : context(context)
        {
            if (context.notify) {
                context.notify->notifyThreadYieldContext(std::this_thread::get_id());
            }
        }
        ~ContextThreadNotifyYield()
        {
            if (context.notify) {
                context.notify->notifyThreadJoinContext(std::this_thread::get_id());
            }
        }
};

}

#if defined(NISSE_HEADER_ONLY) && NISSE_HEADER_ONLY == 1
#endif

#endif
