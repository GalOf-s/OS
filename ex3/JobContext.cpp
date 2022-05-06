#include "JobContext.h"
#include <vector>



//#define TOTAL_COUNT_OFFSET 31
//#define STAGE_OFFSET 62
//
//#define COMPLETED_COUNT_MASK 0x7fffffff // first 31 bit store the number of already processed keys
//#define TOTAL_COUNT_MASK 0x3fffffff80000000 // second 31 bit store number of total keys to process
//#define STAGE_MASK 0x3fffffffffffffff // last  2 bits to flag the stage



JobContext::JobContext(int multiThreadLevel,
                       const MapReduceClient *client,
                       const InputVec *inputVec,
                       OutputVec *outputVec) {
    _multiThreadLevel = multiThreadLevel;
    _mapReduceClient = client;
    _inputVec = inputVec;
    _outputVec = outputVec;
//    _atomic_nextIndex = 0;
//    atomicProgressTracker = _inputVec->size() << TOTAL_COUNT_OFFSET;

    _shuffleStageTotalWork = UNDEFINED_TOTAL_WORK;
    atomicProgressTracker = 0;
	jobState = {UNDEFINED_STAGE, 0};
    _isWaitForJobCalled = false;
    _initWaitForJobMutex();
    _initSaveOutputMutex();
    _initReduceSem(); // TODO check if semaphore is needed
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
        auto jobPair = std::make_pair(this, i);
        if (pthread_create(_threads + i,
                           nullptr,
                           [](void *jobPair){
                               JobContext *obj = static_cast<std::pair<JobContext*, int> *>(jobPair)->first;
                                int i = static_cast<std::pair<JobContext*, int> *>(jobPair)->second;
                                return obj->_run(&obj->_threadContexts[i]);
                               },
                           &jobPair) != 0) {
            _systemError(PTHREAD_CREATE_ERROR);
        }
    }
}

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


void JobContext::_initReduceSem() {
    if (sem_init(_sem_reducePhase, 0, 0) != 0) {
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
		atomicProgressTracker = ((uint64_t) SHUFFLE_STAGE) << STAGE_OFFSET;
        _shufflePhase();

		atomicProgressTracker = ((uint64_t) REDUCE_STAGE) << STAGE_OFFSET;
//        _atomic_nextIndex = 0;
        _wakeUpThreads();
    } else { // TODO: how to make sure this is executed before wake up?
        _reduceSemDown(); // TODO can barrier can be done instead of a semaphore?
    }
    _reducePhase();
    return nullptr;
}

void JobContext::_reduceSemDown() {
    if (sem_wait(_sem_reducePhase) != 0) {
        std::cerr << SYSTEM_ERROR << SEM_DOWN_ERROR << std::endl;
        exit(EXIT_FAILURE);
    }
}

void JobContext::_wakeUpThreads() const {
    for (int i = 0; i < _multiThreadLevel; ++i) {
        if (sem_post(_sem_reducePhase) != 0) {
            std::cerr << SYSTEM_ERROR << SEM_POST_ERROR << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

void JobContext::_mapPhase(ThreadContext *threadContext)
{
	if(atomicProgressTracker < (((uint64_t) MAP_STAGE) << STAGE_OFFSET)) {
		atomicProgressTracker = ((uint64_t) MAP_STAGE) << STAGE_OFFSET;
	}

//    atomicProgressTracker = ((uint64_t) MAP_STAGE) << STAGE_OFFSET;
    unsigned long inputVectorIndex = getNextIndex(atomicProgressTracker += 1 << NEXT_INDEX_OFFSET) - 1;
//    int inputVectorIndex = (_atomic_nextIndex)++;
	emit2Context mapContext = {threadContext, this};
    while(inputVectorIndex < _inputVec->size()) {
        InputPair nextPair = _inputVec->at(inputVectorIndex);
        _mapReduceClient->map(nextPair.first, nextPair.second, &mapContext);
		atomicProgressTracker += 1 << NEXT_INDEX_OFFSET;
//        inputVectorIndex = (_atomic_nextIndex)++;
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
//	_lockMutex(_mutex_saveOutput);
//    jobState.percentage = ((float) _atomic_progressCounter / (float) _inputVec->size())
//                           * 100;
//	_unlockMutex(_mutex_saveOutput);
//
//	(_atomic_inputVectorIndex)++;
//}


void JobContext::_shufflePhase() {
    K2 *maxKey = _getMaxKey();
    while (maxKey){
        IntermediateVec maxVec = _getMaxVec(maxKey);
        _shuffleVec.push_back(maxVec);
        (atomicProgressTracker) += maxVec.size();
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
    unsigned long shuffleVectorIndex = getNextIndex(atomicProgressTracker += 1 << NEXT_INDEX_OFFSET) - 1;
//    int shuffleVectorIndex = (_atomic_nextIndex)++;
    while(shuffleVectorIndex < _shuffleVec.size()) {
        IntermediateVec nextVec = _shuffleVec.at(shuffleVectorIndex);
        _mapReduceClient->reduce(&nextVec,  this);
		atomicProgressTracker += nextVec.size();
		atomicProgressTracker += 1 << NEXT_INDEX_OFFSET;
//        shuffleVectorIndex = (_atomic_nextIndex)++;
    }

	
}

void JobContext::storeReduceResult(OutputPair outputPair) {
    _lockMutex(_mutex_saveOutput);
    _outputVec->push_back(outputPair);
    _unlockMutex(_mutex_saveOutput);
}

void JobContext::_systemError(const std::string &string) {
    std::cerr << SYSTEM_ERROR + string << std::endl;
    exit(EXIT_FAILURE);
}

unsigned long JobContext::getNextIndex(uint64_t progressTrackerValue){
	return progressTrackerValue & (NEXT_INDEX_MASK);
}

unsigned long JobContext::getCompletedCount(u_int64_t progressTrackerValue){
    return progressTrackerValue & (COMPLETED_COUNT_MASK);
}

//unsigned long JobContext::getTotalCount(uint64_t progressTrackerValue){
//    return progressTrackerValue & (TOTAL_COUNT_MASK);
//}

unsigned long JobContext::getState(uint64_t progressTrackerValue){
	return progressTrackerValue & (STAGE_MASK);
}

void JobContext::updateState()
{
	uint64_t currProgressTrackerValue = atomicProgressTracker.load(); // TODO: should it be protected by lock
	jobState.stage = stage_t(getState(currProgressTrackerValue));
	unsigned long completed = getCompletedCount(currProgressTrackerValue);
    unsigned long totalWork = _getTotalWork();
    if(!totalWork) { // total work is 0
        jobState.percentage = 0;
    } else {
        jobState.percentage = 100 * (float) completed / (float) totalWork;

    }
//    unsigned long total = getTotalCount(currProgressTrackerValue);
//    jobState.percentage = 100 * (float) completed / (float) total; // TODO: not always input vector + TODO mutex ?

}

unsigned long JobContext::_getTotalWork(){
	auto currentStage = stage_t(getState(atomicProgressTracker));
	switch (currentStage)
	{
		case UNDEFINED_STAGE:
			return UNDEFINED_TOTAL_WORK;
		case MAP_STAGE:
			return _inputVec->size();
		case SHUFFLE_STAGE:
		case REDUCE_STAGE:
			return _calcShuffleStageTotalWork();
	}
}

unsigned long JobContext::_calcShuffleStageTotalWork() {
	if (_shuffleStageTotalWork == UNDEFINED_TOTAL_WORK)
	{
		int pairs_count = 0;
		for (int i = 0; i < _multiThreadLevel; i++)
		{
			pairs_count += _threadContexts[i].getIntermediateVecSize();
		}
		return _shuffleStageTotalWork = pairs_count;
	}
	else
	{
		return _shuffleStageTotalWork;
	}
}

void JobContext::_destroyMutex(pthread_mutex_t &mutex) {
    if (pthread_mutex_destroy(&mutex) != 0) {
        std::cerr << SYSTEM_ERROR << PTHREAD_MUTEX_DESTROY_ERROR << std::endl;
        exit(EXIT_FAILURE);
    }
}

void JobContext::_destroySem() {
    if (sem_destroy(_sem_reducePhase) < 0) {
        std::cerr << SYSTEM_ERROR << SEM_DESTROY_ERROR << std::endl;
        exit(EXIT_FAILURE);
    }
}

JobContext::~JobContext() {
    _destroyMutex(_mutex_saveOutput);
    _destroyMutex(_mutex_waitForJob);
    _destroySem();
    delete _barrier;
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

void JobContext::joinThreads() {
    for (int i = 0; i < _multiThreadLevel; ++i) {
        if (pthread_join(_threads[i], nullptr) != 0) {
            std::cerr << SYSTEM_ERROR << PTHREAD_JOIN_ERROR << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}
