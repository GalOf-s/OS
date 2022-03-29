//
// Created by gofer on 3/29/2022.
//

#include <sys/time.h>
#include <iostream>
#include "scheduler.h"
#include "ThreadManager.h"

#define USEC_TO_SEC 1000000

Scheduler::Scheduler(int quantum, const ThreadManager& manager){

	struct sigaction sa = {0};
	sa.sa_handler = &Scheduler::switch_thread;
	if (sigaction(SIGVTALRM, &sa, NULL) < 0)
	{
		printf("sigaction error.");
	}
	Scheduler::set_timer();
}


int Scheduler::set_timer(){
	struct itimerval timer;
	timer.it_value.tv_sec = 0;        // first time interval, seconds part
	timer.it_value.tv_usec = 0;        // first time interval, microseconds part

	// configure the timer to expire every 3 sec after that.
	timer.it_interval.tv_sec = quantum / USEC_TO_SEC;    // following time intervals seconds part
	timer.it_interval.tv_usec = quantum % USEC_TO_SEC;    // following time intervals,
	// microseconds part

	// Start a virtual timer. It counts down whenever this process is executing.
	if (setitimer(ITIMER_VIRTUAL, &timer, NULL))
	{
		std::cout << "setitimer error.";
		return -1;
	}

	return 0;
}

void Scheduler::switch_thread(int sig)
{
	if(sig == SIGVTALRM){
		int prev_tid = queue.front();
		uthread_block(prev_tid);
		queue.push(prev_tid);
	}

	queue.pop();
	current_thread = queue.front();
	uthread_resume(current_thread);

}

int Scheduler::add_thread(int id)
{
	queue.push(id);
}



