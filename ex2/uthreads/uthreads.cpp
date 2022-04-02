#include "uthreads.h"
#include "ThreadManager.h"
#include "Scheduler.h"

#include <iostream>


int uthread_init(int quantum_usecs)
{
	ThreadManager::ThreadManager_init(MAX_THREAD_NUM);
	Scheduler::Scheduler_init(quantum_usecs);
	return 0;
}

int uthread_spawn(thread_entry_point entry_point)
{
	Thread new_thread = Thread(entry_point);
	ThreadManager::addNewThread(&new_thread);
	Scheduler::addThread(new_thread.getId());
	return 0;
}

int uthread_terminate(int tid)
{
	return 0;
}

int uthread_block(int tid)
{
	Thread* target_thread = ThreadManager::getThreadById(tid);
	target_thread->block();
	return 0;
}

int uthread_resume(int tid)
{
	Thread* target_thread = ThreadManager::getThreadById(tid);
	target_thread->setState(READY);
	Scheduler::addThread(tid);
	return 0;
}

int uthread_sleep(int num_quantums)
{
	// array of sleeping threads in the scheduler
	return 0;
}

int uthread_get_tid()
{
	return Scheduler::s_currentThreadId;
}

int uthread_get_total_quantums()
{
	return Scheduler::s_totalQuantums;
}

int uthread_get_quantums(int tid)
{

	ThreadManager::getThreadById(tid)->getQuantumsCount();
}

int scheduler_start(){
	return 0;
}
