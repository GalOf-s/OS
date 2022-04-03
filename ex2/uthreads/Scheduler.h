#ifndef UTHREADS_SCHEDULER_H
#define UTHREADS_SCHEDULER_H


#include <sys/time.h>
#include <vector>
#include <algorithm>
#include "ThreadManager.h"


#define MAIN_THREAD_ID 0
#define MIC_TO_SEC 1000000

class Scheduler
{

public:
    static int getCurrentThreadId();

    static int getTotalQuantums();

    static void Scheduler_init(int quantum);

	static int addThreadToReady(int id);

    static int addThreadToSleep(int id, int numQuantums);

    static void setTimer(int quantum);

	static void switchThread(int sig);

    static void startTimer();

    static void removeThreadFromReady(int id);

private:
    static int s_currentThreadId;
    static int s_totalQuantums;
    static std::vector<int> s_readyThreads;
    static std::vector<std::pair<int, int>> s_sleepThreads;
    static struct itimerval timer;
	static Thread* getNextReadyThread();


    static void manageSleepThreads();
};


#endif //UTHREADS_SCHEDULER_H
