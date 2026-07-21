#include "gtest/gtest.h"
#include <latch>
#include <thread>
#include "Server.h"
#include "Pynt.h"

using ThorsAnvil::Nisse::Server::Server;
using ThorsAnvil::Nisse::Server::Pynt;
using ThorsAnvil::Nisse::Server::PyntResult;
using ThorsAnvil::Nisse::Server::Context;

class SocketSetUp
{
#ifdef  __WINNT__
    public:
        SocketSetUp()
        {
            WSADATA wsaData;
            WORD wVersionRequested = MAKEWORD(2, 2);
            int err = WSAStartup(wVersionRequested, &wsaData);
            if (err != 0) {
                printf("WSAStartup failed with error: %d\n", err);
                throw std::runtime_error("Failed to set up Sockets");
            }
        }
        ~SocketSetUp()
        {
            WSACleanup();
        }
#endif
};


/*
 * Some locations were we build do not currently support std::jthread.
 * This is a simplified version just for testing purposes.
 */
//    std::jthread
class LocalJthread: public std::thread
{
    public:
        using std::thread::thread;
        ~LocalJthread()
        {
            join();
        }
};

namespace TASock = ThorsAnvil::ThorsSocket;

class PyntTest: public Pynt
{
    public:
        virtual PyntResult handleRequest(TASock::SocketStream& /*stream*/, Context& /*context*/) override
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2s);
            return PyntResult::Done;
        }
};
PyntTest    testerPynt;


TEST(ServerTest, stopSoft)
{
    SocketSetUp     socketSetup;
    ThorsAnvil::Nisse::Server::UnitTest::ServerRunner<Server>   server;
    server.listen(TASock::ServerInfo{8070}, testerPynt);

    server.stopSoft();
}

TEST(ServerTest, stopHard)
{
    SocketSetUp     socketSetup;
    ThorsAnvil::Nisse::Server::UnitTest::ServerRunner<Server>   server;
    server.listen(TASock::ServerInfo{8070}, testerPynt);

    server.stopHard();
}

TEST(ServerTest, stopSoftWithWork)
{
    SocketSetUp     socketSetup;
    ThorsAnvil::Nisse::Server::UnitTest::ServerRunner<Server>   server;
    server.listen(TASock::ServerInfo{8070}, testerPynt);

    auto action = [&](){
        TASock::SocketStream socketData({"localhost", 8070});
        socketData << "Check" << std::flush;
    };

    LocalJthread    work2(action);

    server.stopSoft();
}


TEST(ServerTest, stopHardWithWork)
{
    SocketSetUp     socketSetup;
    ThorsAnvil::Nisse::Server::UnitTest::ServerRunner<Server>   server;
    server.listen(TASock::ServerInfo{8070}, testerPynt);

    auto action = [&](){
        TASock::SocketStream socketData({"localhost", 8070});
        // I am getting an issue with the LSP telling me the line below is an error.
        // But it compiles fine. If you can explain to me why so I can resolve that would be great.
        // TODO: Understand the LSP error.
        socketData << "Check" << std::flush;
    };
    LocalJthread    work2(action);

    server.stopHard();
}

