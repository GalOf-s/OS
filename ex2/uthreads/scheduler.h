#ifndef UTHREADS_SCHEDULER_H
#define UTHREADS_SCHEDULER_H


#include <queue>
#include "ThreadManager.h"

class Scheduler
{
public:
	Scheduler(int quantum, const ThreadManager &manager);

	int add_thread();
	static int set_timer(int quantum);

	void switch_thread(int sig);

private:
	ThreadManager manager;
	std::queue<int> queue;
	int current_thread{};
	int quantum;
};


#endif //UTHREADS_SCHEDULER_H
