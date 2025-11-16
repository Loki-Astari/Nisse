#include "gtest/gtest.h"
#include <exception>
#include <thread>
#include "NisseServer.h"
#include "Pynt.h"

using ThorsAnvil::Nisse::Server::NisseServer;
using ThorsAnvil::Nisse::Server::Pynt;
using ThorsAnvil::Nisse::Server::PyntResult;
using ThorsAnvil::Nisse::Server::Context;

namespace TASock = ThorsAnvil::ThorsSocket;

class PyntTest: public Pynt
{
    public:
        virtual PyntResult handleRequest(TASock::SocketStream& stream, Context& context) override
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2s);
            return PyntResult::Done;
        }
};
PyntTest    testerPynt;


TEST(NisseServerTest, stopSoft)
{
    NisseServer     server;
    server.listen(TASock::ServerInfo{8070}, testerPynt);

    auto action = [&](){server.run();};
    std::jthread    work(action);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);

    server.stopSoft();
}

TEST(NisseServerTest, stopHard)
{
    NisseServer     server;
    server.listen(TASock::ServerInfo{8070}, testerPynt);

    auto action = [&](){server.run();};
    std::jthread    work(action);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);

    server.stopHard();
}

TEST(NisseServerTest, stopSoftWithWork)
{
    NisseServer     server;
    server.listen(TASock::ServerInfo{8070}, testerPynt);

    auto action1 = [&](){server.run();};
    auto action2 = [&](){
        TASock::SocketStream socketData({"localhost", 8070});
        socketData << "Check" << std::flush;
    };

    std::jthread    work1(action1);
    std::jthread    work2(action2);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);

    server.stopSoft();
}

TEST(NisseServerTest, stopHardWithWork)
{
    NisseServer     server;
    server.listen(TASock::ServerInfo{8070}, testerPynt);

    auto action1 = [&](){server.run();};
    auto action2 = [&](){
        TASock::SocketStream socketData({"localhost", 8070});
        socketData << "Check" << std::flush;
    };
    std::jthread    work1(action1);
    std::jthread    work2(action2);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);

    server.stopHard();
}

