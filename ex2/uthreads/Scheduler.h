#ifndef UTHREADS_SCHEDULER_H
#define UTHREADS_SCHEDULER_H


#include <sys/time.h>
#include <vector>
#include <algorithm>
#include "ThreadManager.h"


#define MIC_TO_SEC 1000000
#define NO_ID -1

#define SYSTEM_ERROR "system error: "
#define SIGPROCMASK_ERROR "sigprocmask failed."
#define SETITIMER_ERROR "setitimer failed."
#define SIGACTION_ERROR "sigaction fail."
#define SIGEMPTYSET_ERROR "sigemptyset failed."
#define SIGADDSET_ERROR "sigaddset failed."

class Scheduler
{

public:

    static int getCurrentThreadId();

    static int getTotalQuantums();

    static void Scheduler_init(int quantum);

	static void addThreadToReadyQueue(int id);

    static void addThreadToSleep(int numQuantums);

    static void setTimer(int quantum);

	static void switchThread(int sig);

    static void blockTimerSig();

    static void unblockTimerSig();

    static void removeThreadFromReady(int id);

    static void setNoRunningThread();

    static sigset_t &getTimerSet();

private:
    static int s_currentThreadId;
    static int s_totalQuantums;
    static std::vector<int> s_readyThreads;
    static std::vector<std::pair<int, int>> s_sleepingThreads;
    static struct itimerval timer;
    static sigset_t timerSet;

    static Thread* getNextReadyThread(); //TODO check if we need this function

    static void _manageSleepThreads();

    static void _startTimer();

    static void _systemError(const std::string& str);

};


#endif //UTHREADS_SCHEDULER_H
