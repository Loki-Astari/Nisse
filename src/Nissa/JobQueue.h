#ifndef THORSANVIL_NISSA_JOB_QUEUE_H
#define THORSANVIL_NISSA_JOB_QUEUE_H

/*
 * The class that holds all the background threads and work that the threads will do.
 *
 * Constructor creates all the child threads.
 * New jobs added via `addJob()` which will then be executed ASAP by one of the threads.
 */

#include "NissaConfig.h"
#include <queue>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace ThorsAnvil::Nissa
{

using Work    = std::function<void()>;
class JobQueue
{
    using WorkQueue     = std::queue<Work>;

    std::vector<std::thread>        workers;
    std::mutex                      workMutex;
    std::condition_variable         workCV;
    WorkQueue                       workQueue;
    bool                            finished;

    public:
        JobQueue(int workerCount);
        ~JobQueue();

        void addJob(Work&& action);

    private:
        Work     getNextJob();
        void     processWork();
};

}

#endif
