#include <sys/time.h>
#include <iostream>
#include <csignal>
#include "Scheduler.h"
#include "ThreadManager.h"



void Scheduler::Scheduler_init(int quantum) {
	s_totalQuantums = 1;
	s_currentThreadId = MAIN_THREAD_ID;
	ThreadManager::getThreadById(MAIN_THREAD_ID)->setState(RUNNING);
	Scheduler::setTimer(quantum);
}


void Scheduler::setTimer(int quantum){

    struct sigaction sa = {0};


    sa.sa_handler = &Scheduler::switchThread;
    if (sigaction(SIGVTALRM, &sa, NULL) < 0)
    {
        printf("sigaction error."); // TODO check if a message should be printed
    }
	timer.it_value.tv_sec = 0;        // first time interval, seconds part
	timer.it_value.tv_usec = 0;        // first time interval, microseconds part

	// configure the timer to expire every 3 sec after that.
	timer.it_interval.tv_sec = quantum / MIC_TO_SEC;    // following time intervals seconds part
	timer.it_interval.tv_usec = quantum % MIC_TO_SEC;    // following time intervals, microseconds part

    startTimer();
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
//	currentThread = queue.front();
//	uthread_resume(currentThread);

    // TODO handle what happens when a thread is sleeping, to send SIGVTALRM ?
	Thread *prevThread = ThreadManager::getThreadById(s_currentThreadId); // the running thread time is up
	int retValue = sigsetjmp(prevThread->env, 1);
	bool switchThread = retValue == 0;
	if (switchThread){
		Thread *currentThread = getNextReadyThread();

		if (currentThread == nullptr) {
			// TODO check what to do if the queue is empty when times up, then no one is ready so the main thread with keep running (?)
			currentThread = ThreadManager::getThreadById(MAIN_THREAD_ID);
		}

		s_currentThreadId = currentThread->getId(); // set current running thread to next in line
		currentThread->setState(RUNNING);
		currentThread->incQuantumCounter();
		s_totalQuantums++;
		manageSleepThreads();


		startTimer(); // starts the timer and jumps to run the thread
		if (sig == SIGVTALRM){ // alarm turned on so switch to the next thread in line
			prevThread->setState(READY); // change the running thread state to ready
			s_readyThreads.push_back(s_currentThreadId); // push the current running thread to the back of the queue


		}
		siglongjmp(currentThread->env, 1);
	}
}

int Scheduler::addThreadToReady(int id)
{
    Thread *targetThread = ThreadManager::getThreadById(id);
    if(targetThread->getState() != READY){
        targetThread->setState(READY);
    }
	s_readyThreads.push_back(id);
}

void Scheduler::startTimer() {

    if (setitimer(ITIMER_VIRTUAL, &timer, NULL))
	{
		std::cout << "setitimer error."; // TODO check if to print
	}
}

Thread *Scheduler::getNextReadyThread()
{
	if(s_readyThreads.empty())
	{
		return nullptr;
	}
	Thread* currentThread;
	do
	{
        s_currentThreadId = s_readyThreads.front();
        s_readyThreads.erase(s_readyThreads.begin());
		currentThread = ThreadManager::getThreadById(s_currentThreadId);
	} while (currentThread->getState() != READY);
	return currentThread;
}

void Scheduler::removeThreadFromReady(int id) {
    s_readyThreads.erase(std::remove(s_readyThreads.begin(), s_readyThreads.end(), id),
                         s_readyThreads.end());
}

int Scheduler::getCurrentThreadId() {
    return s_currentThreadId;
}

int Scheduler::getTotalQuantums() {
    return s_totalQuantums;
}

int Scheduler::addThreadToSleep(int id, int numQuantums) {

    s_sleepingThreads.emplace_back(id, numQuantums + 1); // TODO check if numQuantums +1 needed
}

void Scheduler::manageSleepThreads(){
	auto it = s_sleepingThreads.begin();

	while(it != s_sleepingThreads.end()) {
		it->second--;
		if(it->second == 0) {
			it = s_sleepingThreads.erase(it);
			if(ThreadManager::getThreadById(it->first)->getState() == READY){
				Scheduler::addThreadToReady(it->first);
			}
		}
		else ++it;
	}
}

