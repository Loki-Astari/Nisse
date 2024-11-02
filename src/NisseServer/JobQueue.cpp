#include "JobQueue.h"
#include <ThorsLogging/ThorsLogging.h>

using namespace ThorsAnvil::Nisse::Server;

JobQueue::JobQueue(std::size_t workerCount)
    : finished{false}
{
    try
    {
        for (std::size_t loop = 0; loop < workerCount; ++loop) {
            workers.emplace_back(&JobQueue::processWork, this);
        }
    }
    catch (...)
    {
        stop();
        throw;
    }
}

JobQueue::~JobQueue()
{
    stop();
}

void JobQueue::addJob(Work&& action)
{
    std::unique_lock    lock(workMutex);
    workQueue.emplace(action);
    workCV.notify_one();
}

void JobQueue::markFinished()
{
    std::unique_lock    lock(workMutex);
    finished = true;
}

void JobQueue::stop()
{
    markFinished();
    workCV.notify_all();
    for (auto& w: workers) {
        w.join();
    }
    workers.clear();
}

std::optional<Work> JobQueue::getNextJob()
{
    std::unique_lock    lock(workMutex);
    workCV.wait(lock, [&](){return !workQueue.empty() || finished;});

    if (workQueue.empty() || finished) {
        return {};
    }

    Work work = std::move(workQueue.front());
    workQueue.pop();
    return work;
}

void JobQueue::processWork()
{
    while (!finished)
    {
        std::optional<Work> work   = getNextJob();
        try
        {
            if (work.has_value()) {
                (*work)();
            }
        }
        catch (std::exception const& e)
        {
            ThorsLogWarning("ThorsAnvil::Nissa::JobQueue", "processWork", "Work Exception: ",  e.what());
        }
        catch (...)
        {
            ThorsLogWarning("ThorsAnvil::Nissa::JobQueue", "processWork", "Work Exception: Unknown");
        }
    }
}
