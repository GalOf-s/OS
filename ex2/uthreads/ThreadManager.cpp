#include <vector>
#include <iostream>
#include "ThreadManager.h"
#include "Scheduler.h"


std::vector<Thread*> ThreadManager::s_threads;
int ThreadManager::s_minFreeId;
int ThreadManager::s_maxThreadsNum;

void ThreadManager::ThreadManager_init(int maxThreadsNum) {
    s_maxThreadsNum = maxThreadsNum;
    s_minFreeId = 1;
    s_threads = std::vector<Thread*>(maxThreadsNum, nullptr);
    Thread *mainThread = new Thread();
    if(mainThread == nullptr){
        _systemError(MEMORY_ALLOCATION_ERROR);
    }
    s_threads[MAIN_THREAD_ID] = mainThread; // creates main thread
}


Thread *ThreadManager::getThreadById(int id)
{
	return s_threads[id];
}

int ThreadManager::_generateNewThreadId()
{
	//ThreadManager::deleteTerminatedThreads();

	int curMinFreeId = s_minFreeId;
	for (int i = s_minFreeId + 1; i < s_maxThreadsNum; i++){
		if (s_threads[i] == nullptr){
			s_minFreeId = i;
            break;
		}
	}
	if(s_minFreeId == curMinFreeId){
		s_minFreeId = FAILURE;
	}

	return curMinFreeId;
}

int ThreadManager::addNewThread(thread_entry_point entry_point)
{
	int id = _generateNewThreadId();
    if(id == FAILURE){
        return FAILURE;
    }
    auto *newThread = new Thread(id, entry_point);
    if(newThread == nullptr){
        _systemError(MEMORY_ALLOCATION_ERROR);
    }
    s_threads[id] = newThread;
    return id;
}

int ThreadManager::validateThreadId(int id)
{
	if(id < 0 || MAX_THREAD_ID - 1 < id){
		return FAILURE;
	}
	if (s_threads[id] == nullptr){
		return FAILURE;
	}
	return SUCCESS;
}

void ThreadManager::_deleteThread(int id)
{
    delete s_threads[id];
    s_threads[id] = nullptr;
    if(id < s_minFreeId){
        s_minFreeId = id;
    }
}

void ThreadManager::ThreadManager_destruct() {
    for (Thread *thread : s_threads){
        delete thread;
        thread = nullptr;
    }
}

void ThreadManager::terminate(int threadId) {
    Thread* targetThread = s_threads[threadId];
	if (targetThread->getState() == RUNNING){
//		targetThread->setState(TERMINATED);
        ThreadManager::_deleteThread(threadId);
        Scheduler::setNoRunningThread();
        Scheduler::switchThread(SIGUSR1);
	}else if(targetThread->getState() == READY)
	{
		Scheduler::removeThreadFromReady(threadId); // removes thread from ready queue
	}
    ThreadManager::_deleteThread(threadId);
}

//void ThreadManager::deleteTerminatedThreads()
//{
//	for(int i=0; i<s_maxThreadsNum; i++){
//		if(s_threads[i]->getState() == TERMINATED)
//		{
//			ThreadManager::_deleteThread(i);
//		}
//	}
//}

void ThreadManager::blockThread(int id){
	if (s_threads[id]->getState() == RUNNING){
		s_threads[id]->setState(BLOCKED);
		Scheduler::switchThread(SIGUSR1);
	} else {
        s_threads[id]->setState(BLOCKED);
		Scheduler::removeThreadFromReady(id);
	}
}

void ThreadManager::resumeThread(int id)
{
	Thread* targetThread = ThreadManager::getThreadById(id);
	if (targetThread->getState() == BLOCKED){
		targetThread->setState(READY);
		Scheduler::addThreadToReadyQueue(id);
	}
}

void ThreadManager::sleepThread(int id, int numQuantums){
    ThreadManager::getThreadById(id)->setState(READY);
    Scheduler::addThreadToSleep(numQuantums);
    Scheduler::switchThread(SIGUSR1);
}

void ThreadManager::_systemError(const std::string &string) {
    std::cerr << SYSTEM_ERROR + string << std::endl;
    exit(EXIT_FAILURE);
}
