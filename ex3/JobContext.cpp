#include "JobContext.h"
#include "Barrier.h"
#include <thread>
#include <iostream>

JobContext::JobContext(int multiThreadLevel,
					   const MapReduceClient* client,
					   const InputVec* vec) {
	_multiThreadLevel = multiThreadLevel;
	mapReduceClient = client;
	inputVec = vec;
	atomic_inputVectorIndex = 0;
	atomic_progressCounter = 0;
	jobState = {UNDEFINED_STAGE, 0};
	barrier = new Barrier(_multiThreadLevel);

	try{
		threads = new pthread_t[_multiThreadLevel];
	} catch (std::bad_alloc &) {
		_systemError(MEMORY_ALLOCATION_ERROR);
	}
	threadContexts = std::vector<ThreadContext>(multiThreadLevel);
	_createThreads();
}

void JobContext::_createThreads() {
    for (int i = 0; i < _multiThreadLevel; i++) {
        threadContexts[i] = new ThreadContext(i, (JobContext*) this);
        if (pthread_create(threads + i, nullptr,
                           [](void *obj){ return ((ThreadContext *)obj)->run(); },
                           &threadContexts[i]) != 0) {
            _systemError(PTHREAD_CREATE_ERROR);
        }
    }
}

void JobContext::_systemError(const std::string &string) {
    std::cerr << SYSTEM_ERROR + string << std::endl;
    exit(EXIT_FAILURE);
}

void _systemError(const std::string &string) {
	std::cerr << SYSTEM_ERROR + string << std::endl;
	exit(EXIT_FAILURE);
}

void _lockMutex(pthread_mutex_t &mutex) {
	if (pthread_mutex_lock(&mutex) != 0) {
		_systemError(PTHREAD_MUTEX_LOCK_ERROR);
	}
}

void _unlockMutex(pthread_mutex_t &mutex) { // TODO: declare in beginning
	if (pthread_mutex_unlock(&mutex) != 0) {
		_systemError(PTHREAD_MUTEX_UNLOCK_ERROR);
	}
}
int JobContext::incProgress() // TODO: make the updated values stage dependant
{
	(atomic_progressCounter)++;
	_lockMutex(s_mutex_stagePercentage);
	jobState.percentage = ((float) atomic_progressCounter / (float) inputVec->size())
									   * 100;
	_unlockMutex(s_mutex_stagePercentage);

	return (atomic_inputVectorIndex)++;
}
