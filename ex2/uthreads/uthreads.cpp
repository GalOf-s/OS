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
	int id = ThreadManager::generateNewThreadId();
	Thread new_thread = Thread(entry_point, id);
	Scheduler::addThread(id);
	return 0;
}

int uthread_terminate(int tid)
{
	return 0;
}

int uthread_block(int tid)
{
	return 0;
}

int uthread_resume(int tid)
{
	return 0;
}

int uthread_sleep(int num_quantums)
{
	return 0;
}

int uthread_get_tid()
{
	return 0;
}

int uthread_get_total_quantums()
{
	return 0;
}

int uthread_get_quantums(int tid)
{
	return 0;
}

int scheduler_start(){
	return 0;
}
