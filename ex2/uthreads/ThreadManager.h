#ifndef UTHREADS_THREADMANAGER_H
#define UTHREADS_THREADMANAGER_H

#include <vector>
#include "Thread.h"

#define SUCCESS 0
#define FAILURE -1
#define MAIN_THREAD_ID 0


class ThreadManager
{
public:
    static void ThreadManager_init(int maxThreadsNum);
    static Thread *getThreadById(int id);
	static int addNewThread(thread_entry_point entry_point);
	static int validateThreadId(int id);
    static void ThreadManager_destruct();
    static void terminate(int threadId);
	static void deleteTerminatedThreads();

private:
	static std::vector<Thread*> s_threads;
	static int s_minFreeId;
	static int _maxThreadsNum;
    static int _generateNewThreadId();
    static void _deleteThread(int id);

};


#endif //UTHREADS_THREADMANAGER_H
