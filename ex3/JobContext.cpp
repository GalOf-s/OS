#include "JobContext.h"
#include "Barrier.h"
#include <thread>
#include <semaphore.h>
#include <iostream>



JobContext::JobContext(int multiThreadLevel,
                       const MapReduceClient *client,
                       const InputVec *inputVec,
                       OutputVec *outputVec) {
    _multiThreadLevel = multiThreadLevel;
    _mapReduceClient = client;
    _inputVec = inputVec;
    _outputVec = outputVec;
    _atomic_inputVectorIndex = 0;
    _atomic_progressCounter = 0;
    _atomic_shuffleVectorSize = 0;
    _jobState = {UNDEFINED_STAGE, 0};
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

void *JobContext::_run(void *args)
{
    auto *threadContext = (ThreadContext *) args;
    _jobState.stage = MAP_STAGE; // TODO check if is it ok that all threads do this?
    _mapPhase(threadContext);
    threadContext->sortPhase();
    _barrier->barrier(); // TODO: is it right to init here?
    if(threadContext->getId() == 0) {
        // TODO mutex + update presentage each time
        _jobState = {SHUFFLE_STAGE, 0};
        _atomic_progressCounter = 0;

        _shufflePhase();

        // TODO mutex  + update presentage each time
        _jobState = {REDUCE_STAGE, 0};
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
    int inputVectorIndex = (_atomic_inputVectorIndex)++;
    while(inputVectorIndex < _inputVec->size()){
        InputPair nextPair = _inputVec->at(inputVectorIndex);
        _mapReduceClient->map(nextPair.first, nextPair.second, threadContext);
        _incProgress();
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

void JobContext::_incProgress() // TODO: make the updated values stage dependant
{
	(_atomic_progressCounter)++;
	_lockMutex(_mutex_stagePercentage);
    _jobState.percentage = ((float) _atomic_progressCounter / (float) _inputVec->size())
                           * 100;
	_unlockMutex(_mutex_stagePercentage);

	(_atomic_inputVectorIndex)++;
}


void JobContext::_shufflePhase() {
    K2 *maxKey = _getMaxKey();
    while (!maxKey){
        IntermediateVec maxVec = _getMaxVec(maxKey);
        _shuffleVec.push_back(maxVec);
        (_atomic_shuffleVectorSize)++;
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




