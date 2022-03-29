#ifndef UTHREADS_THREADMANAGER_H
#define UTHREADS_THREADMANAGER_H

#include <vector>
#include "Thread.h"


class ThreadManager
{
public:
	ThreadManager(int maxThreadsNum);

	static std::vector<Thread*> threads;
	static Thread getThreadById(int id);
private:
	static int minFreeId;
	static int max_threads_num;
};


#endif //UTHREADS_THREADMANAGER_H
