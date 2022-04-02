#ifndef UTHREADS_THREADMANAGER_H
#define UTHREADS_THREADMANAGER_H

#include <vector>
#include "Thread.h"


class ThreadManager
{
public:
	static Thread *getThreadById(int id);
    static void ThreadManager_init(int maxThreadsNum);
	static int addNewThread(Thread* thread);
private:
	static std::vector<Thread*> s_threads;
	static int generateNewThreadId();
	static int s_minFreeId;
	static int _maxThreadsNum;
};


#endif //UTHREADS_THREADMANAGER_H
