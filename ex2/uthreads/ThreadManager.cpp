#include <vector>
#include <iostream>
#include "ThreadManager.h"


void ThreadManager::ThreadManager_init(int maxThreadsNum) {
    _maxThreadsNum = maxThreadsNum;
    s_minFreeId = 1;
    s_threads = std::vector<Thread*>(maxThreadsNum, nullptr);
}


Thread *ThreadManager::getThreadById(int id)
{
	return s_threads[id];
}

int ThreadManager::generateNewThreadId()
{
	if (s_minFreeId == -1){
        std::cerr << "thread library error: No more s_threads can be created\n";
		return -1;
	}
	int cur_min_free_id = s_minFreeId;
	for (int i = s_minFreeId + 1; i < _maxThreadsNum; i++){
		if (s_threads[i] == nullptr){
			s_minFreeId = i;
		}
	}
	return cur_min_free_id;
}
