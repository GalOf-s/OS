#ifndef UTHREADS_SCHEDULER_H
#define UTHREADS_SCHEDULER_H


#include <queue>
#include "ThreadManager.h"

class Scheduler
{
public:
	Scheduler(int quantum, const ThreadManager &manager);

	static int addThread(int id);
	static int setTimer();

	static void switchThread(int sig);

private:
	static ThreadManager manager;
	static std::queue<int> queue;
	static int currentThread;
	static int quantum;
	static int totalQuantums;
};


#endif //UTHREADS_SCHEDULER_H
