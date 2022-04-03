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
	if(newThreadId == -1){
        std::cerr << "thread library error: No more s_threads can be created\n";
        return FAILURE;
    }
    Scheduler::addThreadToReady(newThreadId);
	return SUCCESS;
}

int uthread_terminate(int tid)
{
    // TODO what to do if a thread terminate itself?

	if (ThreadManager::validateThreadId(tid) == -1){
		return FAILURE; // TODO print something?
	}
	if(tid == MAIN_THREAD_ID){
        // TODO check in the furom if the only way to exit the program is when terminate(0), if not, the memory wont free
        ThreadManager::ThreadManager_destruct();
		exit(SUCCESS);
	}
    ThreadManager::terminate(tid);
    return SUCCESS;
}

int uthread_block(int tid)
{
	if (ThreadManager::validateThreadId(tid) == -1){
		return FAILURE; // TODO print something?
	}
	Thread* targetThread = ThreadManager::getThreadById(tid);
	targetThread->block();
	return SUCCESS;
}

int uthread_resume(int tid)
{
	if (ThreadManager::validateThreadId(tid) == -1){
		return FAILURE;
	}
	Thread* targetThread = ThreadManager::getThreadById(tid);
	targetThread->resume();
    Scheduler::addThreadToReady(tid);
	return 0;
}

int uthread_sleep(int num_quantums)
{
    if(Scheduler::getCurrentThreadId() == MAIN_THREAD_ID){
        std::cerr << "thread library error: unable to put the main thread to sleep\n";
        return FAILURE;
    }
    int currentThread = Scheduler::getCurrentThreadId();
    Thread* targetThread = ThreadManager::getThreadById(currentThread);
    targetThread->block();
    Scheduler::addThreadToSleep(currentThread, num_quantums);
	return 0;
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
