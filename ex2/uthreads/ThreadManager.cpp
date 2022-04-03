#include <vector>
#include <iostream>
#include "ThreadManager.h"
#include "Scheduler.h"


void ThreadManager::ThreadManager_init(int maxThreadsNum) {
    _maxThreadsNum = maxThreadsNum;
    s_minFreeId = 1;
    s_threads = std::vector<Thread*>(maxThreadsNum, nullptr);
    Thread *mainThread = new Thread();
    if(mainThread == nullptr){
        std::cerr << "system error: failed to allocate memory\n";
        exit(EXIT_FAILURE);
    }
    s_threads[MAX_THREAD_ID] = new Thread(); // creates main thread
}


Thread *ThreadManager::getThreadById(int id)
{
	return s_threads[id];
}

int ThreadManager::_generateNewThreadId()
{
	if (s_minFreeId == -1){
		return -1;
	}
	int curMinFreeId = s_minFreeId;
	for (int i = s_minFreeId + 1; i < _maxThreadsNum; i++){
		if (s_threads[i] == nullptr){
			s_minFreeId = i;
		}
	}
	return curMinFreeId;
}

int ThreadManager::addNewThread(thread_entry_point entry_point)
{
	int id = _generateNewThreadId();
    if(id == -1){
        return -1;
    }
    Thread *newThread = new Thread(id, entry_point);
    if(newThread == nullptr){
        std::cerr << "system error: failed to allocate memory\n";
        exit(EXIT_FAILURE);
    }
    s_threads[id] = newThread;
    return id;
}

int ThreadManager::validateThreadId(int id)
{
	if(id < 0 || MAX_THREAD_ID - 1 < id){
		return -1;
	}
	if (s_threads[id] == nullptr){
		return -1;
	}
	return SUCCESS;
}

void ThreadManager::_deleteThread(int id)
{
    delete s_threads[id];
    s_threads[id] = nullptr;
}

void ThreadManager::ThreadManager_destruct() {
    for (Thread *thread : s_threads){
        delete thread;
        thread = nullptr;
    }
}

void ThreadManager::terminate(int threadId) {
    Thread* targetThread = s_threads[threadId];
    if(targetThread->getState() == READY){
        Scheduler::removeThreadFromReady(threadId); // removes thread from ready queue
    }
    ThreadManager::_deleteThread(threadId);
}


