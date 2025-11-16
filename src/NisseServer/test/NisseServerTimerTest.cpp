#include "gtest/gtest.h"
#include <thread>
#include "NisseServer.h"

using namespace ThorsAnvil::Nisse::Server;

class TestAction: public ThorsAnvil::Nisse::Server::TimerAction
{
    int& value;
    public:
        TestAction(int& value)
            : value(value)
        {}
        virtual void handleRequest(int timerId) override
        {
            value += 5;
        }
};

TEST(NisseServerTimerTest, TestTimer)
{
    int                 value = 0;
    NisseServer         server;
    TestAction          action(value);

    using namespace std::chrono_literals;
    server.addTimer(2s, action);

    auto work = [&server](){server.run();};
    std::jthread    test(work);

    EXPECT_EQ(0, value);
    std::this_thread::sleep_for(1s);
    EXPECT_EQ(0, value);
    std::this_thread::sleep_for(2s);
    EXPECT_EQ(5, value);

    server.stopHard();
}

