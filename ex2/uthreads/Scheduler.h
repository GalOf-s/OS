#ifndef UTHREADS_SCHEDULER_H
#define UTHREADS_SCHEDULER_H


#include <sys/time.h>
#include <queue>
#include "ThreadManager.h"


#define MAIN_THREAD_ID 0
#define MIC_TO_SEC 1000000

class Scheduler
{

public:
    static int s_currentThreadId;
    static int s_totalQuantums;


    static void Scheduler_init(int quantum);

	static int addThread(int id);

	static void setTimer(int quantum);

	static void switchThread(int sig);

    static void startTimer();

private:
    static ThreadManager s_threadManager;
    static std::queue<int> s_queue;
    static struct itimerval timer;
	static Thread* getNextReadyThread();


};


#endif //UTHREADS_SCHEDULER_H
