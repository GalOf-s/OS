#include "JobContext.h"
#include "Barrier.h"
#include <thread>
#include <semaphore.h>
#include <iostream>

#define NEXT_INDEX_OFFSET 31
#define STAGE_OFFSET 62
#define NEXT_INDEX_MASK 0x3fffffff80000000
#define STAGE_MASK 0x3fffffffffffffff
#define COMPLETED_COUNT_MASK 0x7fffffff

JobContext::JobContext(int multiThreadLevel,
                       const MapReduceClient *client,
                       const InputVec *inputVec,
                       OutputVec *outputVec) {
    _multiThreadLevel = multiThreadLevel;
    _mapReduceClient = client;
    _inputVec = inputVec;
    _outputVec = outputVec;
    _atomic_progressTracker = 0;
	jobState = {UNDEFINED_STAGE, 0};
    _initMutex();
    _initSem(); // TODO check if semaphore is needed
    _memoryAllocation();
    _threadContexts = std::vector<ThreadContext>(multiThreadLevel);
	_createThreads();
}

void JobContext::_memoryAllocation() { // TODO better method name? (:
    try{
        _barrier = new Barrier(_multiThreadLevel);
        _threads = new pthread_t[_multiThreadLevel];
    } catch (std::bad_alloc &) {
        _systemError(MEMORY_ALLOCATION_ERROR);
    }
}

void JobContext::_createThreads() {
    for (int i = 0; i < _multiThreadLevel; i++) {
        _threadContexts[i] = ThreadContext(i);
        if (pthread_create(_threads + i,
                           nullptr,
                           [](void *obj){ return ((JobContext *)obj)->_run(obj); },
                           &_threadContexts[i]) != 0) {
            _systemError(PTHREAD_CREATE_ERROR);
        }
    }
}

void JobContext::_initMutex() {
    if (pthread_mutex_init(&_mutex_stagePercentage, nullptr) != 0) {
        _systemError(PTHREAD_MUTEX_INIT_ERROR);
    }
}

void JobContext::_initSem() {
    if (sem_init(&_sem_reducePhase, 0, 0) != 0) {
        std::cerr << SYSTEM_ERROR << SEM_INIT_ERROR << std::endl;
        exit(EXIT_FAILURE);
    }
}

void *JobContext::_run(void *inputThreadContext)
{
	auto threadContext = (ThreadContext *) inputThreadContext;
    _mapPhase(threadContext);
    threadContext->sortPhase();
    _barrier->barrier();

    if(threadContext->getId() == 0) {
		_atomic_progressTracker = ((uint64_t) SHUFFLE_STAGE) << STAGE_OFFSET;
        _shufflePhase();

		_atomic_progressTracker = ((uint64_t) REDUCE_STAGE) << STAGE_OFFSET;
        _wakeUpThreads();
    } else {
        _reduceSemDown(); // TODO can barrier can be done instead of a semaphore?
    }
    _reducePhase();
}

void JobContext::_reduceSemDown() {
    if (sem_down(&_sem_reducePhase) != 0) {
        std::cerr << SYSTEM_ERROR << SEM_DOWN_ERROR << std::endl;
        exit(EXIT_FAILURE);
    }
}

void JobContext::_wakeUpThreads() const {
    for (int i = 0; i < _multiThreadLevel; ++i) {
        if (sem_post(&_sem_reducePhase) != 0) {
            std::cerr << SYSTEM_ERROR << SEM_POST_ERROR << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

void JobContext::_mapPhase(ThreadContext *threadContext)
{
	if(_atomic_progressTracker < (((uint64_t) MAP_STAGE) << STAGE_OFFSET)){
		_atomic_progressTracker += ((uint64_t) MAP_STAGE) << STAGE_OFFSET;
	}
    int inputVectorIndex = getNextIndex(_atomic_progressTracker += (1 + (1 << NEXT_INDEX_OFFSET ))) - 1;
    while(inputVectorIndex < _inputVec->size()){
        InputPair nextPair = _inputVec->at(inputVectorIndex);
        _mapReduceClient->map(nextPair.first, nextPair.second, threadContext);
        _atomic_progressTracker += ((1 << NEXT_INDEX_OFFSET) + 1);
    }
}


void JobContext::_lockMutex(pthread_mutex_t &mutex) {
	if (pthread_mutex_lock(&mutex) != 0) {
		_systemError(PTHREAD_MUTEX_LOCK_ERROR);
	}
}


void JobContext::_unlockMutex(pthread_mutex_t &mutex) { // TODO: declare in beginning
	if (pthread_mutex_unlock(&mutex) != 0) {
		_systemError(PTHREAD_MUTEX_UNLOCK_ERROR);
	}
}

//void JobContext::_incProgress() // TODO: make the updated values stage dependant
//{
//	(_atomic_progressCounter)++;
//	_lockMutex(_mutex_stagePercentage);
//    jobState.percentage = ((float) _atomic_progressCounter / (float) _inputVec->size())
//                           * 100;
//	_unlockMutex(_mutex_stagePercentage);
//
//	(_atomic_inputVectorIndex)++;
//}


void JobContext::_shufflePhase() {
    K2 *maxKey = _getMaxKey();
    while (maxKey){
        IntermediateVec maxVec = _getMaxVec(maxKey);
        _shuffleVec.push_back(maxVec);
        (_atomic_progressTracker)++;
        maxKey = _getMaxKey();
    }
}


K2* JobContext::_getMaxKey() {
    K2* maxKey;
    for (auto & _threadContext : _threadContexts){
        if (!_threadContext.isIntermediateVecEmpty() && (maxKey == nullptr || *maxKey < *_threadContext.getMaxKey()))
        {
            *maxKey = *_threadContext.getMaxKey();
        }
    }
    return maxKey;
}


IntermediateVec JobContext::_getMaxVec(K2 *maxKey) {
    IntermediateVec maxVec;
    for (auto & _threadContext : _threadContexts){
        while(!_threadContext.isIntermediateVecEmpty()  && _equalKeys(maxKey, _threadContext.getMaxKey())) {
            maxVec.push_back(_threadContext.getMaxPair());
        }
    }
    return maxVec;
}

bool JobContext::_equalKeys(K2 *firstKey, K2 *secondKey) {
    return !(*firstKey < *secondKey) && !(*secondKey < *firstKey); // if true the keys are equal
}

void JobContext::_reducePhase() {

}

void JobContext::storeReduceResult(OutputPair outputPair) {
    _outputVec->push_back(outputPair);
}

void JobContext::_systemError(const std::string &string) {
    std::cerr << SYSTEM_ERROR + string << std::endl;
    exit(EXIT_FAILURE);
}

int JobContext::getNextIndex(u_int64_t progressTrackerValue){
	return (int) progressTrackerValue & (NEXT_INDEX_MASK);
}

int JobContext::getState(u_int64_t progressTrackerValue){
	return (int) progressTrackerValue & (STAGE_MASK);
}

int JobContext::getCompletedCount(u_int64_t progressTrackerValue){
	return (int) progressTrackerValue & (COMPLETED_COUNT_MASK);
}

void JobContext::updateState()
{
	uint64_t currProgressTrackerValue = _atomic_progressTracker.load(); // TODO: should it be protected by lock
	jobState.stage = stage_t(getState(currProgressTrackerValue));
	int completed = getCompletedCount(currProgressTrackerValue);
	jobState.percentage = 100 * (float)(completed / _inputVec->size()); // TODO: not always input vector
}
