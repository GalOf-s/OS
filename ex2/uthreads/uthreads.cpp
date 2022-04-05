#include "uthreads.h"
#include "ThreadManager.h"
#include "Scheduler.h"

#include <iostream>

#define SUCCESS 0
#define FAILURE -1


int uthread_init(int quantum_usecs)
{
    if(quantum_usecs < 0){
        std::cerr << "thread library error: quantum should be a positive number\n";
        return FAILURE;
    }
	ThreadManager::ThreadManager_init(MAX_THREAD_ID);
	Scheduler::Scheduler_init(quantum_usecs);
	return SUCCESS;
}

int uthread_spawn(thread_entry_point entry_point)
{
    int newThreadId = ThreadManager::addNewThread(entry_point);
	if(newThreadId == FAILURE){
        std::cerr << "thread library error: No more s_threads can be created\n";
        return FAILURE;
    }
	Scheduler::addThreadToReadyQueue(newThreadId);
	return SUCCESS;
}

int uthread_terminate(int tid)
{
	if (ThreadManager::validateThreadId(tid) == FAILURE){
		return FAILURE; // TODO print something?
	}
	if(tid == MAIN_THREAD_ID){
        // TODO check in the furom if the only way to exit the program is when terminate(0), if not, the memory wont free
        ThreadManager::ThreadManager_destruct();
		exit(EXIT_SUCCESS);
	}

    ThreadManager::terminate(tid);
    return SUCCESS;
}

int uthread_block(int tid)
{
    if (ThreadManager::validateThreadId(tid) == FAILURE){
        return FAILURE; // TODO print something?
    }
	ThreadManager::blockThread(tid);
    return SUCCESS;
}

int uthread_resume(int tid)
{
    if (ThreadManager::validateThreadId(tid) == FAILURE){
        return FAILURE; // TODO print something?
    }
	ThreadManager::resumeThread(tid);
    return SUCCESS;
}

int uthread_sleep(int num_quantums)
{
	int currentThreadId = Scheduler::getCurrentThreadId();
	if(currentThreadId == MAIN_THREAD_ID){
		std::cerr << "thread library error: unable to put the main thread to sleep\n";
		return FAILURE;
	}
    ThreadManager::sleepThread(num_quantums);
	return SUCCESS;
}

int uthread_get_tid()
{
	return Scheduler::getCurrentThreadId();
}

int uthread_get_total_quantums()
{
	return Scheduler::getTotalQuantums();
}

int uthread_get_quantums(int tid)
{
	if (ThreadManager::validateThreadId(tid) == -1){
		return FAILURE;
	}
	ThreadManager::getThreadById(tid)->getQuantumsCount();
    return SUCCESS;
}

int scheduler_start(){
	return 0;
}
