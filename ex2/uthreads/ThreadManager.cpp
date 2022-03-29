#include <vector>
#include <iostream>
#include "ThreadManager.h"

Thread ThreadManager::getThreadById(int id)
{
	return *threads[id];
}

ThreadManager::ThreadManager(int maxThreadsNum)
{
	maxThreadsNum = maxThreadsNum;
	minFreeId = 1;
	threads = std::vector<Thread*>(maxThreadsNum, nullptr);

}

int ThreadManager::generateNewThreadId()
{
	if(minFreeId == -1){
		std::cerr << "thread library error: No more threads can be created\n";
		return -1;
	}
	int cur_min_free_id = minFreeId;
	for(int i=minFreeId+1; i<maxThreadsNum; i++){
		if(threads[i] == nullptr){
			minFreeId = i;
		}
	}
	return cur_min_free_id;
}
