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

CoRoutine Server::buildCoRoutineStream(StreamData& info)
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
                info.task(info.stream, yield);
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

StreamTask Server::createStreamJob(Pint& pint)
{
    // When an accepted socket is available.
    // Run this function. Which simply delegates the work to the pint.
    // If the pint return PintResult::More then yield back to the server
    // indicating you are waiting for more data to read when it arrives
    // re-call the pint.
    return [&pint](ThorsAnvil::ThorsSocket::SocketStream& stream, Yield& yield)
    {
        PintResult result = pint.handleRequest(stream);
        while (result == PintResult::More)
        {
            yield(TaskYieldState::RestoreRead);
            result = pint.handleRequest(stream);
        }
    };
}

CoRoutine Server::buildCoRoutineServer(ServerData& info)
{
    return CoRoutine
    {
        [&info](Yield& yield)
        {
            // Set the socket to work asynchronously.
            info.server.setYield([&yield](){yield(TaskYieldState::RestoreRead);return true;});

            // Return control to the creator.
            // The next call will happen when there is data available on the file descriptor.
            yield(TaskYieldState::RestoreRead);

            try
            {
                info.task(info.server, yield);
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

ServerTask Server::createAcceptJob(Pint& pint)
{
    // When a connection is accepted
    // This method is run.
    return [&](ThorsAnvil::ThorsSocket::Server& server, Yield& yield)
    {
        while (true)
        {
            using ThorsAnvil::ThorsSocket::Socket;
            using ThorsAnvil::ThorsSocket::Blocking;

            Socket          accept = server.accept(Blocking::No);
            if (accept.isConnected())
            {
                // If everything worked then create a stream connection (see above)
                // Passing the "Pint" as the object that will handle the request.
                // Note: The "Pint" functionality is not run yet. The socket must be available to use.
                eventHandler.add(ThorsAnvil::ThorsSocket::SocketStream{std::move(accept)}, createStreamJob(pint), [&](StreamData& info){return buildCoRoutineStream(info);});
            }
            yield(TaskYieldState::RestoreRead);
        }
    };
}

template<typename T>
void Server::listen(T listenerInit, Pint& pint)
{
    using ThorsAnvil::ThorsSocket::Server;
    Server  server{listenerInit};

    eventHandler.add(std::move(server), createAcceptJob(pint), [&](ServerData& info){return buildCoRoutineServer(info);});
}

template void Server::listen<ThorsAnvil::ThorsSocket::SServerInfo>(ThorsAnvil::ThorsSocket::SServerInfo listenerInit, Pint& pint);
template void Server::listen<ThorsAnvil::ThorsSocket::ServerInfo>(ThorsAnvil::ThorsSocket::ServerInfo listenerInit, Pint& pint);
