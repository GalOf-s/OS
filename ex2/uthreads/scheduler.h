#ifndef UTHREADS_SCHEDULER_H
#define UTHREADS_SCHEDULER_H


#include <queue>
#include "ThreadManager.h"

class Scheduler
{
public:
	Scheduler(int quantum, const ThreadManager &manager);

	static int add_thread(int id);
	static int set_timer();

	static void switch_thread(int sig);

private:
	static ThreadManager manager;
	static std::queue<int> queue;
	static int current_thread;
	static int quantum;
};


#endif //UTHREADS_SCHEDULER_H
