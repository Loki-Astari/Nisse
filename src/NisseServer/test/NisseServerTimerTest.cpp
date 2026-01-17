#include "gtest/gtest.h"
#include <thread>
#include "NisseServer.h"

using namespace ThorsAnvil::Nisse::Server;

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

class TestAction: public ThorsAnvil::Nisse::Server::TimerAction
{
    int& value;
    public:
        TestAction(int& value)
            : value(value)
        {}
        virtual void handleRequest(int /*timerId*/) override
        {
            value += 5;
        }
};

TEST(NisseServerTimerTest, TestTimer)
{
#if 0
    int                 value = 0;
    TestAction          action(value);
    NisseServer         server;
    std::latch          latch(1);

    using namespace std::chrono_literals;
    server.addTimer(2s, action);

    auto work = [&server, &latch](){server.run([&latch](){latch.count_down();});};
    LocalJthread    test(work);

    EXPECT_EQ(0, value);
    std::this_thread::sleep_for(1s);
    EXPECT_EQ(0, value);
    std::this_thread::sleep_for(2s);
    EXPECT_EQ(5, value);

    latch.wait();
    server.stopHard();
#endif
}

