#include <vector>
#include "ThreadManager.h"

Thread ThreadManager::getThreadById(int id)
{
	return *threads[id];
}

ThreadManager::ThreadManager(int maxThreadsNum)
{
	max_threads_num = maxThreadsNum;
	minFreeId = 1;
	threads = std::vector<Thread*>(max_threads_num);

}
