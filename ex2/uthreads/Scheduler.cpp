#include <sys/time.h>
#include <iostream>
#include <csignal>
#include "Scheduler.h"
#include "ThreadManager.h"



void Scheduler::Scheduler_init(int quantum) {
    Scheduler::setTimer(quantum);
    s_currentThreadId = MAIN_THREAD_ID;
    ThreadManager::getThreadById(MAIN_THREAD_ID)->setState(RUNNING);
	s_totalQuantums = 1;
}


void Scheduler::setTimer(int quantum){

    struct sigaction sa = {0};


    sa.sa_handler = &Scheduler::switchThread;
    if (sigaction(SIGVTALRM, &sa, NULL) < 0)
    {
        printf("sigaction error."); // TODO check if a message should be printed
    }
	timer.it_value.tv_sec = quantum / MIC_TO_SEC;        // first time interval, seconds part
	timer.it_value.tv_usec = quantum % MIC_TO_SEC;        // first time interval, microseconds part

	// configure the timer to expire every 3 sec after that.
	timer.it_interval.tv_sec = 0;    // following time intervals seconds part
	timer.it_interval.tv_usec = 0;    // following time intervals, microseconds part

    startTimer();

	// Start a virtual timer. It counts down whenever this process is executing.
//	if (setitimer(ITIMER_VIRTUAL, &timer, NULL))
//	{
//		std::cout << "setitimer error.";
//		return -1;
//	}
//
//	return 0;


}

void Scheduler::switchThread(int sig)
{
//	if(sig == SIGVTALRM){
//		int prevTimeId = queue.front();
//		uthread_block(prevTimeId); // why?
//		queue.push(prevTimeId);
//	}
//
//	queue.pop();
//	nextCurrentThread = queue.front();
//	uthread_resume(nextCurrentThread);

    // if the queue is empty when times up, then no one is ready so the main thread with keep running (?)
    // TODO check if ok
    if (s_queue.empty()) {
        return;
    }

	Thread *prevThread = ThreadManager::getThreadById(s_currentThreadId); // the running thread time is up
	int retValue = sigsetjmp(prevThread->env, 1);
	bool switchThread = retValue == 0;

	if (sig == SIGVTALRM && switchThread){ // alarm turned on so switch to the next thread in line
		prevThread->setState(READY); // change the running thread state to ready
		s_queue.push(s_currentThreadId); // push the current running thread t
	}

	Thread *nextCurrentThread = getNextReadyThread();
	s_currentThreadId = nextCurrentThread->getId(); // set current running thread to next in line
	nextCurrentThread->setState(RUNNING);
	nextCurrentThread->incQuantumCounter();
	s_totalQuantums++;
	startTimer(); // starts the timer and jumps to run the thread
	siglongjmp(nextCurrentThread->env, 1);

}

int Scheduler::addThread(int id)
{
	s_queue.push(id);
}

void Scheduler::startTimer() {
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL))
	{
		std::cout << "setitimer error."; // TODO check if to print
	}
}

Thread *Scheduler::getNextReadyThread()
{
	Thread* currentThread;
	do
	{
		s_currentThreadId = s_queue.front();
		s_queue.pop();
		currentThread = ThreadManager::getThreadById(s_currentThreadId);
	} while (currentThread->getState() != READY);
	return currentThread;
}

