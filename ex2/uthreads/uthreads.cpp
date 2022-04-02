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
        return FAILURE
    }
    Thread::ThreadInitMain();
	ThreadManager::ThreadManager_init(MAX_THREAD_NUM);
	Scheduler::Scheduler_init(quantum_usecs);
	return SUCCESS;
}

int uthread_spawn(thread_entry_point entry_point)
{
	Thread new_thread = Thread(entry_point);
	ThreadManager::addNewThread(&new_thread);
    Scheduler::addThreadToReady(new_thread.getId());
	return 0;
}

int uthread_terminate(int tid)
{
    // TODO what to do if a thread terminate itself.
	if (ThreadManager::validateThreadId(tid) == -1){
		return FAILURE;
	}

	if(tid == MAIN_THREAD_ID){
		// TODO: release memory;
		exit(SUCCESS);
	}

    Thread* target_thread = ThreadManager::getThreadById(tid);
    target_thread->terminate();
}

int uthread_block(int tid)
{
	if (ThreadManager::validateThreadId(tid) == -1){
		return FAILURE;
	}
	Thread* target_thread = ThreadManager::getThreadById(tid);
	target_thread->block();
	return 0;
}

int uthread_resume(int tid)
{
	if (ThreadManager::validateThreadId(tid) == -1){
		return FAILURE;
	}
	Thread* target_thread = ThreadManager::getThreadById(tid);
	target_thread->resume();
    Scheduler::addThreadToReady(tid);
	return 0;
}

int uthread_sleep(int num_quantums)
{
    if(Scheduler::getCurrentThreadId() == MAIN_THREAD_ID){
        return FAILURE;
    }
    int currentThread = Scheduler::getCurrentThreadId();
    Thread* target_thread = ThreadManager::getThreadById(currentThread);
    target_thread->block();
    Scheduler::addThreadToSleep(currentThread, num_quantums);
	// TODO manage the thread sleep list
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
}

int scheduler_start(){
	return 0;
}
