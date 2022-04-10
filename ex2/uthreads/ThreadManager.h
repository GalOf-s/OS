#ifndef UTHREADS_THREADMANAGER_H
#define UTHREADS_THREADMANAGER_H

#include <vector>
#include <string>
#include "Thread.h"

#define SUCCESS 0
#define FAILURE -1
#define MAIN_THREAD_ID 0

#define SYSTEM_ERROR "system error: "
#define MEMORY_ALLOCATION_ERROR "failed to allocate memory."



class ThreadManager
{
public:
    static void ThreadManager_init(int maxThreadsNum);
    static Thread *getThreadById(int id);
	static int addNewThread(thread_entry_point entry_point);
	static int validateThreadId(int id);
    static void ThreadManager_destruct();
    static void terminate(int threadId);
	//static void deleteTerminatedThreads();
	static void blockThread(int id);
	static void resumeThread(int id);
    static void sleepThread(int id, int numQuantums);

private:
	static std::vector<Thread*> s_threads;
	static int s_minFreeId;
	static int s_maxThreadsNum;

    static int _generateNewThreadId();
    static void _deleteThread(int id);
    static void _systemError(const std::string& string);

};


#endif //UTHREADS_THREADMANAGER_H
