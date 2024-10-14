#include "Server.h"
#include "EventHandler.h"

using namespace ThorsAnvil::Nissa;

Server::Server(int workerCount)
    : jobQueue{workerCount}
    , store{}
    , eventHandler{jobQueue, store}
{}

void Server::run()
{
    eventHandler.run();
}

CoRoutine Server::createStreamJob(StreamData& info)
{
    return CoRoutine
    {
        [&info](Yield& yield)
        {
            // Set the socket to work asynchronously.
            info.stream.getSocket().setReadYield([&yield](){yield(TaskYieldState::RestoreRead);return true;});
            info.stream.getSocket().setWriteYield([&yield](){yield(TaskYieldState::RestoreWrite);return true;});

            // Return control to the creator.
            // The next call will happen when there is data available on the file descriptor.
            yield(TaskYieldState::RestoreRead);

            try
            {
                PintResult result = info.pint->handleRequest(info.stream);
                while (result == PintResult::More)
                {
                    yield(TaskYieldState::RestoreRead);
                    result = info.pint->handleRequest(info.stream);
                }
            }
            catch (...)
            {
                std::cerr << "Pint Exception:\n";
            }
            // We are all done
            // So indicate that we should tidy up state.
            yield(TaskYieldState::Remove);
        }
    };
}

CoRoutine Server::createAcceptJob(ServerData& info)
{
    return CoRoutine
    {
        [&](Yield& yield)
        {
            // Set the socket to work asynchronously.
            info.server.setYield([&yield](){yield(TaskYieldState::RestoreRead);return true;});

            // Return control to the creator.
            // The next call will happen when there is data available on the file descriptor.
            yield(TaskYieldState::RestoreRead);

            try
            {
                while (true)
                {
                    using ThorsAnvil::ThorsSocket::Socket;
                    using ThorsAnvil::ThorsSocket::Blocking;

                    Socket          accept = info.server.accept(Blocking::No);
                    if (accept.isConnected())
                    {
                        // If everything worked then create a stream connection (see above)
                        // Passing the "Pint" as the object that will handle the request.
                        // Note: The "Pint" functionality is not run yet. The socket must be available to use.
                        eventHandler.add(ThorsAnvil::ThorsSocket::SocketStream{std::move(accept)}, [&](StreamData& info){return createStreamJob(info);}, *info.pint);
                    }
                    yield(TaskYieldState::RestoreRead);
                }
            }
            catch (...)
            {
                std::cerr << "Pint Exception:\n";
            }
            // We are all done
            // So indicate that we should tidy up state.
            yield(TaskYieldState::Remove);
        }
    };
}

template<typename T>
void Server::listen(T listenerInit, Pint& pint)
{
    using ThorsAnvil::ThorsSocket::Server;
    Server  server{listenerInit};

    eventHandler.add(std::move(server), [&](ServerData& info){return createAcceptJob(info);}, pint);
}

template void Server::listen<ThorsAnvil::ThorsSocket::SServerInfo>(ThorsAnvil::ThorsSocket::SServerInfo listenerInit, Pint& pint);
template void Server::listen<ThorsAnvil::ThorsSocket::ServerInfo>(ThorsAnvil::ThorsSocket::ServerInfo listenerInit, Pint& pint);
