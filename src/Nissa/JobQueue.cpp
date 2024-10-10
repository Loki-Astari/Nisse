#include "JobQueue.h"

using namespace ThorsAnvil::Nissa;

JobQueue::JobQueue(int workerCount)
    : finished(false)
{
    for (int loop = 0; loop < workerCount; ++loop) {
        workers.emplace_back(&JobQueue::processWork, this);
    }
}

JobQueue::~JobQueue()
{
    for (auto& worker:  workers) {
        worker.join();
    }
}

void JobQueue::addJob(WorkAction&& action, ThorsAnvil::ThorsSocket::SocketStream&& stream)
{
    std::unique_lock    lock(workMutex);
    workQueue.emplace(std::move(action), std::move(stream));
    workCV.notify_one();
}

Work JobQueue::getNextJob()
{
    std::unique_lock    lock(workMutex);
    workCV.wait(lock, [&](){return !workQueue.empty();});
    Work work = std::move(workQueue.front());
    workQueue.pop();
    return work;
}

void JobQueue::processWork()
{
    while (!finished)
    {
        Work work   = getNextJob();
        work.action(std::move(work.stream));
    }
}
