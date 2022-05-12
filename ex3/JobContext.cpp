#include "JobContext.h"
#include <vector>
#include <pthread.h>




JobContext::JobContext(int multiThreadNum,
                       const MapReduceClient *client,
                       const InputVec *inputVector,
                       OutputVec *outputVector) {
    multiThreadLevel = multiThreadNum;
    mapReduceClient = client;
    inputVec = inputVector;
    outputVec = outputVector;
    atomic_nextIndex = 0;
    atomicProgressTracker = inputVec->size() << TOTAL_COUNT_OFFSET;
    atomicProgressTracker = 0;
    atomicTotalPairsCount = 0;
    jobState = {UNDEFINED_STAGE, 0};
    _isWaitForJobCalled = false;
    _initWaitForJobMutex();
    _initSaveOutputMutex();
    _initUpdateStateMutex();
    _memoryAllocation();
}

void JobContext::_memoryAllocation() { // TODO better method name? (:
    try{
        threads = new pthread_t[multiThreadLevel];
        barrier = new Barrier(multiThreadLevel);
        threadContexts = new std::vector<ThreadContext*>();
        shuffleVec = new std::vector<IntermediateVec *>();
    } catch (std::bad_alloc &) {
        _systemError(MEMORY_ALLOCATION_ERROR_MSG);
    }
}

void JobContext::_initWaitForJobMutex() {
    if (pthread_mutex_init(&_mutex_waitForJob, nullptr) != 0) {
        _systemError(PTHREAD_MUTEX_INIT_ERROR_MSG);
    }
}


void JobContext::_initSaveOutputMutex() {
    if (pthread_mutex_init(&mutex_saveOutput, nullptr) != 0) {
        _systemError(PTHREAD_MUTEX_INIT_ERROR_MSG);
    }
}

void JobContext::_initUpdateStateMutex() {
    if (pthread_mutex_init(&mutex_updateState, nullptr) != 0) {
        _systemError(PTHREAD_MUTEX_INIT_ERROR_MSG);
    }
}


void JobContext::_lockMutex(pthread_mutex_t &mutex) {
    if (pthread_mutex_lock(&mutex) != 0) {
        _systemError(PTHREAD_MUTEX_LOCK_ERROR_MSG);
    }
}

void JobContext::_unlockMutex(pthread_mutex_t &mutex) { // TODO: declare in beginning
    if (pthread_mutex_unlock(&mutex) != 0) {
        _systemError(PTHREAD_MUTEX_UNLOCK_ERRO_MSG);
    }
}

void JobContext::makeMaxVec(IntermediateVec *maxVec, K2 *maxKey) const {
    for (auto & threadContext : *threadContexts){
        while(!threadContext->isIntermediateVecEmpty() && _equalKeys(threadContext->getMaxKey(), maxKey)) {
            maxVec->push_back(threadContext->getMaxPair());
        }
    }
}

K2 *JobContext::getMaxKey() const {
    K2* maxKey = nullptr;
    bool isMaxEmpty = true;
    for (auto & threadContext : *threadContexts){
        if (!threadContext->isIntermediateVecEmpty() && (isMaxEmpty || *maxKey < *threadContext->getMaxKey()))
        {
            maxKey = threadContext->getMaxKey();
            isMaxEmpty = false;
        }
    }
    return maxKey;
}

bool JobContext::_equalKeys(K2 *firstKey, K2 *secondKey) {
    return !(*firstKey < *secondKey) && !(*secondKey < *firstKey); // if true the keys are equal
}

void JobContext::storeReduceResult(K3 *key, V3 *value) {
    _lockMutex(mutex_saveOutput);
    OutputPair outputPair(key, value);
    outputVec->push_back(outputPair);
    _unlockMutex(mutex_saveOutput);
}

void JobContext::_systemError(const std::string &string) {
    std::cerr << SYSTEM_ERROR_MSG + string << std::endl;
    exit(EXIT_FAILURE);
}

unsigned long JobContext::_getCompletedCount(u_int64_t progressTrackerValue){
    return progressTrackerValue & COMPLETED_COUNT_MASK;
}

unsigned long JobContext::_getTotalCount(uint64_t progressTrackerValue){
    return (progressTrackerValue & TOTAL_COUNT_MASK) >> TOTAL_COUNT_OFFSET;
}

unsigned long JobContext::_getState(uint64_t progressTrackerValue){
    return (progressTrackerValue & STAGE_MASK) >> STAGE_OFFSET;
}

void JobContext::updateState()
{
    uint64_t currProgressTrackerValue = atomicProgressTracker.load();
    jobState.stage = stage_t(_getState(currProgressTrackerValue));
    unsigned long completed = _getCompletedCount(currProgressTrackerValue);
    unsigned long totalWork = _getTotalCount(currProgressTrackerValue);
    if(!totalWork) { // total work is 0
        jobState.percentage = 0;
    } else {
        jobState.percentage = ROUND_FACTOR_PERCENTAGE * (float)  completed / (float) totalWork;
    }
}

void JobContext::_destroyMutex(pthread_mutex_t &mutex) {
    if (pthread_mutex_destroy(&mutex) != 0) {
        std::cerr << SYSTEM_ERROR_MSG << PTHREAD_MUTEX_DESTROY_ERROR_MSG << std::endl;
        exit(EXIT_FAILURE);
    }
}

bool JobContext::wasWaitForJobCalled() {
    _lockMutex(_mutex_waitForJob);
    if(_isWaitForJobCalled){
        _unlockMutex(_mutex_waitForJob);
        return true;
    }
    _isWaitForJobCalled = true;
    _unlockMutex(_mutex_waitForJob);
    return false;
}

void JobContext::joinThreads() const {
    for (int i = 0; i < multiThreadLevel; ++i) {
        if (pthread_join(threads[i], nullptr) != 0) {
            std::cerr << SYSTEM_ERROR_MSG << PTHREAD_JOIN_ERROR << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

JobContext::~JobContext() {
    _destroyMutex(mutex_saveOutput);
    _destroyMutex(_mutex_waitForJob);
    _destroyMutex(mutex_updateState);
    delete barrier;
    for (auto & threadContext : *threadContexts){
        delete threadContext;
    }
    delete shuffleVec;
    delete threadContexts;
    delete[] threads;
}
