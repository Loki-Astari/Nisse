#ifndef THORSANVIL_NISSE_JOB_QUEUE_H
#define THORSANVIL_NISSE_JOB_QUEUE_H

/*
 * The class that holds all the background threads and work that the threads will do.
 *
 * Constructor creates all the child threads.
 * New jobs added via `addJob()` which will then be executed ASAP by one of the threads.
 */

#include "NisseConfig.h"
#include <queue>
#include <vector>
#include <functional>
#include <optional>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace ThorsAnvil::Nisse
{

using Work    = std::function<void()>;
class JobQueue
{
    std::vector<std::thread>        workers;
    std::mutex                      workMutex;
    std::condition_variable         workCV;
    std::queue<Work>                workQueue;
    bool                            finished;

    public:
        JobQueue(int workerCount);
        ~JobQueue();

        void addJob(Work&& action);
        void stop();

    private:
        std::optional<Work> getNextJob();
        void                processWork();
        void                markFinished();
};

}

#endif
