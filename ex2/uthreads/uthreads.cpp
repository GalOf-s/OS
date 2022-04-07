#include "uthreads.h"
#include "ThreadManager.h"
#include "Scheduler.h"

#include <iostream>

#define SUCCESS 0
#define FAILURE -1

#define THREAD_LIBRARY_ERROR "thread library error: "
#define NEGATIVE_QUANTUM_ERROR "quantum should be a positive number."
#define MAX_THREAD_ERROR "no more threads can be created."
#define NULL_ENTRY_POINT_ERROR "thread entry point can't be NULL."
#define THREAD_ID_ERROR "thread ID is invalid."
#define ILLEGAL_BLOCK_ERROR "unable to block the main thread."
#define ILLEGAL_SLEEP_ERROR "unable to put the main thread tp sleep."


int threadLibraryError(const std::string &string) {
    std::cerr << THREAD_LIBRARY_ERROR + string << std::endl;
    Scheduler::unblockTimerSig();
    return FAILURE;
}

int uthread_init(int quantum_usecs)
{
    if(quantum_usecs <= 0){
        return threadLibraryError(NEGATIVE_QUANTUM_ERROR);
    }
	ThreadManager::ThreadManager_init(MAX_THREAD_ID);
	Scheduler::Scheduler_init(quantum_usecs);
	return SUCCESS;
}

int uthread_spawn(thread_entry_point entry_point)
{
    Scheduler::blockTimerSig();
    if (entry_point == nullptr) {
        return threadLibraryError(NULL_ENTRY_POINT_ERROR);
    }
    int newThreadId = ThreadManager::addNewThread(entry_point);
	if(newThreadId == FAILURE){
        return threadLibraryError(MAX_THREAD_ERROR);
    }
	Scheduler::addThreadToReadyQueue(newThreadId);
    Scheduler::unblockTimerSig();
	return SUCCESS;
}

int uthread_terminate(int tid)
{
    Scheduler::blockTimerSig();
	if (ThreadManager::validateThreadId(tid) == FAILURE){
		return threadLibraryError(THREAD_ID_ERROR);
	}
	if(tid == MAIN_THREAD_ID){
        Scheduler::unblockTimerSig();
        ThreadManager::ThreadManager_destruct();
		exit(EXIT_SUCCESS);
	}
    ThreadManager::terminate(tid);
    Scheduler::unblockTimerSig();
    return SUCCESS;
}

int uthread_block(int tid)
{
    Scheduler::blockTimerSig();
    if (ThreadManager::validateThreadId(tid) == FAILURE){
        return threadLibraryError(THREAD_ID_ERROR);
    }
    if (tid == MAIN_THREAD_ID){
        return threadLibraryError(ILLEGAL_BLOCK_ERROR);
    }
    if (ThreadManager::getThreadById(tid)->getState() == BLOCKED){
        return SUCCESS;
    }
	ThreadManager::blockThread(tid);
    Scheduler::unblockTimerSig();
    return SUCCESS;
}

int uthread_resume(int tid)
{
    Scheduler::blockTimerSig();
    if (ThreadManager::validateThreadId(tid) == FAILURE){
        return threadLibraryError(THREAD_ID_ERROR);
    }
    if (ThreadManager::getThreadById(tid)->getState() == BLOCKED){
        ThreadManager::resumeThread(tid);
    }
    Scheduler::unblockTimerSig();
    return SUCCESS;
}

int uthread_sleep(int num_quantums)
{
    Scheduler::blockTimerSig();
	int currentThreadId = Scheduler::getCurrentThreadId();
	if(currentThreadId == MAIN_THREAD_ID){
        return threadLibraryError(ILLEGAL_SLEEP_ERROR);
	}
    ThreadManager::sleepThread(num_quantums);
    Scheduler::unblockTimerSig();
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
    Scheduler::blockTimerSig();
	if (ThreadManager::validateThreadId(tid) == -1){
        return threadLibraryError(THREAD_ID_ERROR);
	}
	int threadQuantums = ThreadManager::getThreadById(tid)->getQuantumsCount();
    Scheduler::unblockTimerSig();
    return threadQuantums;
}
