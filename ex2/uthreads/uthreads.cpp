#include "uthreads.h"
#include "ThreadManager.h"
#include "scheduler.h"

#include <iostream>

extern ThreadManager threadManager;
extern Scheduler scheduler;

int uthread_init(int quantum_usecs)
{
	threadManager= ThreadManager(MAX_THREAD_NUM);
	scheduler = Scheduler(quantum_usecs, threadManager);
	return 0;
}

int uthread_spawn(thread_entry_point entry_point)
{
	int id = threadManager.generateNewThreadId();
	Thread new_thread = Thread(entry_point, id);
	scheduler.addThread(id);
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
