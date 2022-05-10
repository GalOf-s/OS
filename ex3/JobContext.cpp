#include "JobContext.h"
#include <vector>
#include <pthread.h>




JobContext::JobContext(int multiThreadLevel,
                       const MapReduceClient *client,
                       const InputVec *inputVec,
                       OutputVec *outputVec) {
    _multiThreadLevel = multiThreadLevel;
    _mapReduceClient = client;
    _inputVec = inputVec;
    _outputVec = outputVec;
    _atomic_nextIndex = 0;
    atomicProgressTracker = _inputVec->size() << TOTAL_COUNT_OFFSET;


//    _shuffleStageTotalWork = UNDEFINED_TOTAL_WORK;
    atomicProgressTracker = 0;
    atomicTotalPairsCount = 0;
    jobState = {UNDEFINED_STAGE, 0};
    _isWaitForJobCalled = false;
    _memoryAllocation();
//    _createThreads();
}

void JobContext::_memoryAllocation() { // TODO better method name? (:
    try{
        _threads = new pthread_t[_multiThreadLevel];

        _barrier = new Barrier(_multiThreadLevel);
        _threadContexts = new std::vector<ThreadContext*>();
        _shuffleVec = new std::vector<IntermediateVec *>();


    } catch (std::bad_alloc &) {
        _systemError(MEMORY_ALLOCATION_ERROR);
    }
}
//
//void JobContext::_createThreads() {
//    _threadContexts->reserve(_multiThreadLevel);
//    for (int i = 0; i < _multiThreadLevel; i++) {
//        _threadContexts->emplace_back(new ThreadContext(i, this));
//        auto jobPair = std::make_pair(this, i);
//        if (pthread_create(_threads + i*sizeof(pthread_t*),
//                           nullptr,
//                           [](void *jobPair){
//                               JobContext *obj = static_cast<std::pair<JobContext*, int> *>(jobPair)->first;
//                               int i = static_cast<std::pair<JobContext*, int> *>(jobPair)->second;
//                               return obj->_run(i);
//                           },
//                           &jobPair) != 0) {
//            _systemError(PTHREAD_CREATE_ERROR);
//        }
//    }
//}

void JobContext::_initWaitForJobMutex() {
    if (pthread_mutex_init(&_mutex_waitForJob, nullptr) != 0) {
        _systemError(PTHREAD_MUTEX_INIT_ERROR);
    }
}


void JobContext::_initSaveOutputMutex() {
    if (pthread_mutex_init(&_mutex_saveOutput, nullptr) != 0) {
        _systemError(PTHREAD_MUTEX_INIT_ERROR);
    }
}

void JobContext::_initUpdateStateMutex() {
    if (pthread_mutex_init(&_mutex_updateState, nullptr) != 0) {
        _systemError(PTHREAD_MUTEX_INIT_ERROR);
    }
}

void JobContext::_initReduceSem() {
    if (sem_init(&_sem_reducePhase, 0, 0) != 0) {
        std::cerr << SYSTEM_ERROR << SEM_INIT_ERROR << std::endl;
        exit(EXIT_FAILURE);
    }
}


IntermediateVec JobContext::getMaxVec(K2 *maxKey) const{
    IntermediateVec maxVec;
    for (auto & _threadContext : *_threadContexts){
        while(!_threadContext->isIntermediateVecEmpty()  && !(*_threadContext->getMaxKey() < *maxKey)) {
            maxVec.push_back(_threadContext->getMaxPair());
        }
    }
    return maxVec;
}

K2* JobContext::getMaxKey() const {
    K2* maxKey = nullptr;
    for (auto & _threadContext : *_threadContexts){
        if (!_threadContext->isIntermediateVecEmpty() && (maxKey == nullptr || *maxKey < *_threadContext->getMaxKey()))
        {
            maxKey = _threadContext->getMaxKey();
        }
    }
    return maxKey;
}






//
//void *JobContext::_run(int threadId)
//{
//    auto threadContext = _threadContexts->at(threadId);
//    _mapPhase(threadContext);
//    threadContext->sortPhase();
//    std::cout << "size: " << threadContext->_intermediateVec->size() << std::endl;
//
//    _barrier->barrier();
//
//    if(threadContext->getId() == 0) {
//        atomicProgressTracker = (((uint64_t) SHUFFLE_STAGE) << STAGE_OFFSET) + (atomicTotalPairsCount << TOTAL_COUNT_OFFSET);
//        std::cout<<"shuffling"<<std::endl;
//        _shufflePhase();
//        std::cout<<"finished shuffling"<<std::endl;
//
//        atomicProgressTracker = (((uint64_t) REDUCE_STAGE) << STAGE_OFFSET) + (atomicTotalPairsCount << TOTAL_COUNT_OFFSET);
//        atomicTotalPairsCount = 0;
//        _atomic_nextIndex = 0;
//        _wakeUpThreads(_sem_reducePhase);
//        threadContext->deleteIntermediateVec();
//    } else { // TODO: how to make sure this is executed before wake up?
//        _reduceSemDown(); // TODO can barrier can be done instead of a semaphore?
//        threadContext->deleteIntermediateVec();
//    }
//    _reducePhase();
//    return nullptr;
//}

void JobContext::_reduceSemDown() {
    if (sem_wait(&_sem_reducePhase) != 0) {
        std::cerr << SYSTEM_ERROR << SEM_DOWN_ERROR << std::endl;
        exit(EXIT_FAILURE);
    }
}

void JobContext::_wakeUpThreads(sem_t &sem) const {
    for (int i = 0; i < _multiThreadLevel; ++i) {
        if (sem_post(&sem) != 0) {
            std::cerr << SYSTEM_ERROR << SEM_POST_ERROR << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}
//
//void JobContext::_mapPhase(ThreadContext *threadContext)
//{
//    _lockMutex(_mutex_updateState);
//    if(atomicProgressTracker.load() < (((uint64_t) MAP_STAGE) << STAGE_OFFSET)) {
//        atomicProgressTracker = ((uint64_t) MAP_STAGE << STAGE_OFFSET);
//    }
//    _unlockMutex(_mutex_updateState);
//
//
//    unsigned long inputVectorIndex = (_atomic_nextIndex)++;
//    unsigned long inputVectorLength = _inputVec->size();
//    emit2Context *mapContext;
//    while(inputVectorIndex < inputVectorLength) {
////        InputPair nextPair = _inputVec->at(inputVectorIndex);
////        emit2Context mapContext = {threadContext, this};
////        _mapReduceClient->map(nextPair.first, nextPair.second, &mapContext);
//
////        mapContext = new emit2Context{this, threadContext->getId()};
//        _mapReduceClient->map(_inputVec->at(inputVectorIndex).first, _inputVec->at(inputVectorIndex).second, threadContext);
//        delete mapContext;
//        inputVectorIndex = (_atomic_nextIndex)++;
//        atomicProgressTracker++; // update count of completed input pairs
//    }
//}


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

//void JobContext::_shufflePhase() {
//    K2 *maxKey = _getMaxKey();
//    while (maxKey){
//        IntermediateVec maxVec = _getMaxVec(maxKey);
//        _shuffleVec.push_back(maxVec);
//        (atomicProgressTracker) += maxVec.size();
//        maxKey = _getMaxKey();
//    }
//}


//K2* JobContext::_getMaxKey() const {
//    K2* maxKey = nullptr;
//    for (auto & _threadContext : *_threadContexts){
//        if (!_threadContext->isIntermediateVecEmpty() && (maxKey == nullptr || *maxKey < *_threadContext->getMaxKey()))
//        {
//            maxKey = _threadContext->getMaxKey();
//        }
//    }
//    return maxKey;
//}
//
//
//IntermediateVec JobContext::_getMaxVec(K2 *maxKey) const {
//    IntermediateVec maxVec;
//    for (auto & _threadContext : *_threadContexts){
//        while(!_threadContext->isIntermediateVecEmpty()  && !(*_threadContext->getMaxKey() < *maxKey)) {
//            maxVec.push_back(_threadContext->getMaxPair());
//        }
//    }
//    return maxVec;
//}
//
//bool JobContext::_equalKeys(K2 *firstKey, K2 *secondKey) {
//    return !(*firstKey < *secondKey) && !(*secondKey < *firstKey); // if true the keys are equal
//}
//
//void JobContext::_reducePhase() {
//    unsigned long shuffleVectorIndex = (_atomic_nextIndex)++;
//    unsigned long shuffleVectorLength = _shuffleVec.size();
//
//    while(shuffleVectorIndex < shuffleVectorLength) {
//        IntermediateVec nextVec = _shuffleVec.at(shuffleVectorIndex);
//        _mapReduceClient->reduce(&nextVec,  this);
//        atomicProgressTracker += nextVec.size();
//        shuffleVectorIndex = (_atomic_nextIndex)++;
//
//    }
//}

//void JobContext::storeReduceResult(OutputPair outputPair) {
//    _lockMutex(_mutex_saveOutput);
//    _outputVec->push_back(outputPair);
//    _unlockMutex(_mutex_saveOutput);
//}

void JobContext::_systemError(const std::string &string) {
    std::cerr << SYSTEM_ERROR + string << std::endl;
    exit(EXIT_FAILURE);
}

unsigned long JobContext::getCompletedCount(u_int64_t progressTrackerValue){
    return progressTrackerValue & COMPLETED_COUNT_MASK;
}

unsigned long JobContext::getTotalCount(uint64_t progressTrackerValue){
    return (progressTrackerValue & TOTAL_COUNT_MASK) >> TOTAL_COUNT_OFFSET;
}

unsigned long JobContext::getState(uint64_t progressTrackerValue){
    return (progressTrackerValue & STAGE_MASK) >> STAGE_OFFSET;
}

//void JobContext::updateState()
//{
//    uint64_t currProgressTrackerValue = atomicProgressTracker.load(); // TODO: should it be protected by lock
//    jobState.stage = stage_t(getState(currProgressTrackerValue));
//    unsigned long completed = getCompletedCount(currProgressTrackerValue);
//    unsigned long totalWork = getTotalCount(currProgressTrackerValue);
//    if(!totalWork) { // total work is 0
//        jobState.percentage = 0;
//    } else {
//        jobState.percentage = 100.0f * (float)  completed / (float) totalWork;
//
//    }
//}

void JobContext::_destroyMutex(pthread_mutex_t &mutex) {
    if (pthread_mutex_destroy(&mutex) != 0) {
        std::cerr << SYSTEM_ERROR << PTHREAD_MUTEX_DESTROY_ERROR << std::endl;
        exit(EXIT_FAILURE);
    }
}

void JobContext::_destroySem() {
    if (sem_destroy(&_sem_reducePhase) < 0) {
        std::cerr << SYSTEM_ERROR << SEM_DESTROY_ERROR << std::endl;
        exit(EXIT_FAILURE);
    }
}

JobContext::~JobContext() {
    _destroyMutex(_mutex_saveOutput);
    _destroyMutex(_mutex_waitForJob);
    _destroyMutex(_mutex_updateState);
    _destroySem();
    delete _barrier;
    for (auto & _threadContext : *_threadContexts){
        delete _threadContext;
    }
    delete _threadContexts;
    delete[] _threads;
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
    for (int i = 0; i < _multiThreadLevel; ++i) {
        if (pthread_join(_threads[i], nullptr) != 0) {
            std::cerr << SYSTEM_ERROR << PTHREAD_JOIN_ERROR << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}
