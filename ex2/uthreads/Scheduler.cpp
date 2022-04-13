#include <sys/time.h>
#include <iostream>
#include <csignal>
#include "Scheduler.h"
#include "ThreadManager.h"


int Scheduler::s_currentThreadId;
int Scheduler::s_totalQuantums;
std::vector<int> Scheduler::s_readyThreads;
std::vector <std::pair<int, int>> Scheduler::s_sleepingThreads;
struct itimerval Scheduler::timer;
sigset_t Scheduler::timerSet;


void Scheduler::Scheduler_init(int quantum) {
	s_totalQuantums = 1;
	s_currentThreadId = MAIN_THREAD_ID;
	ThreadManager::getThreadById(MAIN_THREAD_ID)->setState(RUNNING);
	_setTimer(quantum);
}


void Scheduler::_setTimer(int quantum){
    if (sigemptyset(&timerSet) < 0) {
        _systemError(SIGEMPTYSET_ERROR);
    }
    if (sigaddset(&timerSet, SIGVTALRM) < 0) {
        _systemError(SIGADDSET_ERROR);
    }

    struct sigaction sa = {0};

    sa.sa_handler = &Scheduler::switchThread;
    if (sigaction(SIGVTALRM, &sa, NULL) < 0)
    {
        _systemError(SIGACTION_ERROR);
    }
	timer.it_value.tv_sec = quantum / MIC_TO_SEC;        // first time interval, seconds part
	timer.it_value.tv_usec = quantum % MIC_TO_SEC;        // first time interval, microseconds part

	timer.it_interval.tv_sec = 0;    // following time intervals seconds part
	timer.it_interval.tv_usec = 0;    // following time intervals, microseconds part

    _startTimer();
}

void Scheduler::switchThread(int sig)
{
    if(s_currentThreadId != NO_ID){ // if a tread didn't get terminated while running
        Thread *prevThread = ThreadManager::getThreadById(s_currentThreadId); // the running thread time is up
        if(sig == SIGVTALRM){ // if a tread didn't get blocked while running
            prevThread->setState(READY); // change the running thread state to ready
            s_readyThreads.push_back(s_currentThreadId); // push the current running thread to the back of the queue
        }
        int retValue = sigsetjmp(prevThread->env, 1);
        if(retValue == 1){
            return;
        }
    }
    Thread *currentThread = _getNextReadyThread();
    if (currentThread == nullptr) {
        currentThread = ThreadManager::getThreadById(MAIN_THREAD_ID);
    }

    s_currentThreadId = currentThread->getId(); // set current running thread to next in line
    currentThread->setState(RUNNING);
    currentThread->incQuantumCounter();
    s_totalQuantums++;
    _manageSleepThreads();

    _startTimer(); // starts the timer and jumps to run the thread
    siglongjmp(currentThread->env, 1);
}

void Scheduler::addThreadToReadyQueue(int id){
    Thread *targetThread = ThreadManager::getThreadById(id);
    if(targetThread->getState() != READY){
        targetThread->setState(READY);
    }
	s_readyThreads.push_back(id);
}

void Scheduler::_startTimer(){
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL))
	{
        ThreadManager::ThreadManager_destruct();
        _systemError(SETITIMER_ERROR);
	}
}

Thread *Scheduler::_getNextReadyThread(){
	if(s_readyThreads.empty())
	{
		return nullptr;
	}
    s_currentThreadId = s_readyThreads.front();
    s_readyThreads.erase(s_readyThreads.begin());
    Thread* currentThread = ThreadManager::getThreadById(s_currentThreadId);
	return currentThread;
}

void Scheduler::removeThreadFromReady(int id) {
    s_readyThreads.erase(std::remove(s_readyThreads.begin(), s_readyThreads.end(), id),
                         s_readyThreads.end());
}

void Scheduler::removeThreadFromSleeping(int id){
    auto it = s_sleepingThreads.begin();

    while(it != s_sleepingThreads.end()) {
        if(it->first == id) {
            it = s_sleepingThreads.erase(it);
            break;
        }
        else ++it;
    }
}

int Scheduler::getCurrentThreadId() {
    return s_currentThreadId;
}

int Scheduler::getTotalQuantums() {
    return s_totalQuantums;
}

void Scheduler::addThreadToSleep(int numQuantums) {
    s_sleepingThreads.emplace_back(s_currentThreadId, numQuantums);
}

void Scheduler::_manageSleepThreads(){
	auto it = s_sleepingThreads.begin();

	while(it != s_sleepingThreads.end()) {
		it->second--;
		if(it->second == 0) {
			if(ThreadManager::getThreadById(it->first)->getState() == READY){
				addThreadToReadyQueue(it->first);
			}
            it = s_sleepingThreads.erase(it);
        }
		else ++it;
	}
}

bool Scheduler::isIdSleeping(int id) {
    for (auto & s_sleepingThread : s_sleepingThreads) {
        if (s_sleepingThread.first == id) {
            return true;
        }
    }
    return false;
}

void Scheduler::setNoRunningThread() {
    s_currentThreadId = NO_ID;
}


void Scheduler::blockTimerSig() {
    if (sigprocmask(SIG_BLOCK, &timerSet, NULL) < 0) {
        ThreadManager::ThreadManager_destruct();
        _systemError(SIGPROCMASK_ERROR);
    }
}

void Scheduler::unblockTimerSig() {
    if (sigprocmask(SIG_UNBLOCK, &timerSet, nullptr) < 0) {
        ThreadManager::ThreadManager_destruct();
        _systemError((std::string &) SIGPROCMASK_ERROR);
    }
}

void Scheduler::_systemError(const std::string &string) {
    std::cerr << SYSTEM_ERROR + string << std::endl;
    exit(EXIT_FAILURE);
}


