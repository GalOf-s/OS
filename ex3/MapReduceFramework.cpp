#include <iostream>
#include "MapReduceFramework.h"
#include "ThreadContext.h"
#include "JobContext.h"
#include <pthread.h>
#include <algorithm>


#define SYSTEM_ERROR "system error: "
#define MEMORY_ALLOCATION_ERROR "failed to allocate memory."

void systemError(const std::string &string);

void lockMutex(pthread_mutex_t &mutex);

void unlockMutex(pthread_mutex_t &mutex);


void wakeUpThreads(JobContext *jobContext, sem_t *sem) {
    for (int i = 0; i < jobContext->_multiThreadLevel; ++i) {
        if (sem_post(sem) != 0) {
            std::cerr << SYSTEM_ERROR << SEM_POST_ERROR << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

//void reduceSemDown(JobContext *jobContext) {
//    if (sem_wait(jobContext->_sem_reducePhase) != 0) {
//        std::cerr << SYSTEM_ERROR << SEM_DOWN_ERROR << std::endl;
//        exit(EXIT_FAILURE);
//    }
//}


void systemError(const std::string &string) {
    std::cerr << SYSTEM_ERROR + string << std::endl;
    exit(EXIT_FAILURE);
}


void lockMutex(pthread_mutex_t &mutex) {
    if (pthread_mutex_lock(&mutex) != 0) {
        systemError(PTHREAD_MUTEX_LOCK_ERROR);
    }
}


void unlockMutex(pthread_mutex_t &mutex) {
    if (pthread_mutex_unlock(&mutex) != 0) {
        systemError(PTHREAD_MUTEX_UNLOCK_ERROR);
    }
}

void mapPhase(ThreadContext *threadContext)
{
    auto jobContext = threadContext->getJobContext();
    lockMutex(jobContext->_mutex_updateState);
    if(jobContext->atomicProgressTracker.load() < (((uint64_t) MAP_STAGE) << STAGE_OFFSET)) {
        jobContext->atomicProgressTracker = ((uint64_t) MAP_STAGE << STAGE_OFFSET);
    }
    unlockMutex(jobContext->_mutex_updateState);


    unsigned long inputVectorIndex = (jobContext->_atomic_nextIndex)++;
    unsigned long inputVectorLength = jobContext->_inputVec->size();
    while(inputVectorIndex < inputVectorLength) {
        jobContext->_mapReduceClient->map(jobContext->_inputVec->at(inputVectorIndex).first, jobContext->_inputVec->at(inputVectorIndex).second, threadContext);
        inputVectorIndex = (jobContext->_atomic_nextIndex)++;
        jobContext->atomicProgressTracker++; // update count of completed input pairs
    }
}

//K2* getMaxKey(ThreadContext *tc) {
//    K2* maxKey = nullptr;
//    for (auto & _threadContext : *(tc->_jobContext->_threadContexts)){
//        if (!_threadContext->isIntermediateVecEmpty() && (maxKey == nullptr || *maxKey < *_threadContext->getMaxKey()))
//        {
//            maxKey = _threadContext->getMaxKey();
//        }
//    }
//    return maxKey;
//}

//IntermediateVec getMaxVec(ThreadContext *tc, JobContext *jobContext, K2 *maxKey){
//    IntermediateVec maxVec;
//    for (auto & _threadContext : *jobContext->_threadContexts){
//        while(!_threadContext->isIntermediateVecEmpty()  && !(*_threadContext->getMaxKey() < *maxKey)) {
//            maxVec.push_back(_threadContext->getMaxPair());
//        }
//    }
//    return maxVec;
//}

K2* getMaxKey(JobContext *jobContext){
    K2* maxKey = nullptr;
    for (auto & tContext : *jobContext->_threadContexts){
        if (!tContext->isIntermediateVecEmpty() && (maxKey == nullptr || *maxKey < *tContext->getMaxKey()))
        {
            maxKey = tContext->getMaxKey();
        }
    }
    return maxKey;
}


void getMaxVec(JobContext *jobContext, K2 *maxKey, IntermediateVec *maxVec){
    for (auto & tContext : *jobContext->_threadContexts){
        while(!tContext->isIntermediateVecEmpty()  && !(*tContext->getMaxKey() < *maxKey)) {
            maxVec->push_back(tContext->getMaxPair());
        }
    }
}


void shufflePhase(JobContext *jobContext) {
    K2 *maxKey = getMaxKey(jobContext);
    while (maxKey){
        auto *maxVec = new IntermediateVec();
        getMaxVec(jobContext, maxKey, maxVec);
        jobContext->_shuffleVec->push_back(maxVec);
        jobContext->atomicProgressTracker += maxVec->size();
        maxKey = getMaxKey(jobContext);
    }
}


void reducePhase(JobContext *jobContext) {
    unsigned long shuffleVectorIndex = jobContext->_atomic_nextIndex++;
    unsigned long shuffleVectorLength = jobContext->_shuffleVec->size();

    while(shuffleVectorIndex < shuffleVectorLength) {
        IntermediateVec *nextVec = jobContext->_shuffleVec->at(shuffleVectorIndex);
        jobContext->_mapReduceClient->reduce(nextVec, jobContext);
        jobContext->atomicProgressTracker += nextVec->size();
//        std::cout << "size: " << nextVec->size() << std::endl;
//		jobContext->atomicProgressTracker++;
		shuffleVectorIndex = jobContext->_atomic_nextIndex++;
    }

}

bool pairComparer(IntermediatePair pair1, IntermediatePair pair2){
    return *pair1.first < *pair2.first;
}


void *run(void *args)
{
    auto threadContext = (ThreadContext *) args;
    auto jobContext = threadContext->getJobContext();
    mapPhase(threadContext);
//    threadContext->sortPhase();
    std::sort(threadContext->_intermediateVec->begin(), threadContext->_intermediateVec->end(), pairComparer);
//    std::cout << "size: " << threadContext->_intermediateVec->size() << std::endl;

    jobContext->_barrier->barrier();

    if(threadContext->getId() == 0) {
        jobContext->atomicProgressTracker = (((uint64_t) SHUFFLE_STAGE) << STAGE_OFFSET) + (jobContext->atomicTotalPairsCount << TOTAL_COUNT_OFFSET);
//        std::cout<<"shuffling"<<std::endl;
        shufflePhase(jobContext);
//        std::cout<<"finished shuffling"<<std::endl;

        jobContext->_atomic_nextIndex = 0;
        jobContext->atomicProgressTracker = (((uint64_t) REDUCE_STAGE) << STAGE_OFFSET) + (jobContext->atomicTotalPairsCount << TOTAL_COUNT_OFFSET);
        jobContext->atomicTotalPairsCount = 0;
        jobContext->_wakeUpThreads(jobContext->_sem_reducePhase);
//        threadContext->deleteIntermediateVec();
        delete threadContext->_intermediateVec;

    }
//    jobContext->_barrier->barrier();
    else { // TODO: how to make sure this is executed before wake up?
        jobContext->_reduceSemDown(); // TODO can barrier can be done instead of a semaphore?
        delete threadContext->_intermediateVec;
    }
    reducePhase(jobContext);
    return nullptr;
}

void createThreads(JobContext *jobContext) {
    jobContext->_threadContexts->reserve(jobContext->_multiThreadLevel);
    for (int i = 0; i < jobContext->_multiThreadLevel; i++) {
        jobContext->_threadContexts->emplace_back(new ThreadContext(i, jobContext));
        if (pthread_create(jobContext->_threads + i, nullptr, run, jobContext->_threadContexts->at(i)) != 0) {
            systemError(PTHREAD_CREATE_ERROR);
        }
    }
}

unsigned long getCompletedCount(u_int64_t progressTrackerValue){
    return progressTrackerValue & COMPLETED_COUNT_MASK;
}

unsigned long getTotalCount(uint64_t progressTrackerValue){
    return (progressTrackerValue & TOTAL_COUNT_MASK) >> TOTAL_COUNT_OFFSET;
}

unsigned long getState(uint64_t progressTrackerValue){
    return (progressTrackerValue & STAGE_MASK) >> STAGE_OFFSET;
}


void updateState(JobContext *jobContext)
{
    lockMutex(jobContext->_mutex_updateState);
    uint64_t currProgressTrackerValue = jobContext->atomicProgressTracker.load(); // TODO: should it be protected by lock
    unlockMutex(jobContext->_mutex_updateState);
    jobContext->jobState.stage = stage_t(getState(currProgressTrackerValue));
    unsigned long completed = getCompletedCount(currProgressTrackerValue);
    unsigned long totalWork = getTotalCount(currProgressTrackerValue);
    if(!totalWork) { // total work is 0
        jobContext->jobState.percentage = 0;
    } else {
        jobContext->jobState.percentage = 100.0f * (float)  completed / (float) totalWork;

    }
}




/**
 * This function starts running the MapReduce algorithm (with several _threads)
 * and returns a JobHandle.
 *
 * @param client The task that the framework should run.
 * @param inputVec The input elements.
 * @param outputVec An empty vector to which the output elements
 * will be added before returning.
 * @param multiThreadLevel A valid number of worker _threads to be used for running the algorithm.
 * @return JobHandle that will be used for monitoring the job.
 */
JobHandle startMapReduceJob (const MapReduceClient& client,
                             const InputVec& inputVec,
                             OutputVec& outputVec,
                             int multiThreadLevel) {
    JobContext *jobContext;
    try{
        jobContext = static_cast<JobContext *>(new JobContext(multiThreadLevel,
                                                              &client,
                                                              &inputVec,
                                                              &outputVec));
        createThreads(jobContext);
    } catch (std::bad_alloc &) {
        systemError(MEMORY_ALLOCATION_ERROR);
    }
    return jobContext;
}

/**
 * Gets JobHandle returned by startMapReduceFramework and waits until it is finished.
 *
 * @param job A running job.
 */
void waitForJob(JobHandle job) {
    auto *jobContext = (JobContext *) job;
    if(jobContext->wasWaitForJobCalled()) {
        return;
    }
    jobContext->joinThreads();
}

/**
 * Gets a JobHandle and updates the state of the job into the given JobState struct.
 *
 * @param job A running job.
 * @param state State to update.
 */
void getJobState(JobHandle job, JobState* state) {
    auto jobContext = (JobContext *) job;
    updateState(jobContext);
    *state = jobContext->jobState;
}


/**
 * Releasing all resources of a job.
 * Note: in case that the function is called and the job is not finished yet
 * wait until the job is finished to close it.
 *
 * @param job A running job.
 */
void closeJobHandle(JobHandle job) {
    waitForJob(job);
    auto *jobContext = (JobContext *) job;
    delete jobContext;
}

/**
 * Receives as input intermediary element (K2, V2) and context which contains data
 * structure of the thread that created the intermediary element.
 * Saves the intermediary element in the context data structures.
 * In addition, updates the number of intermediary elements using atomic counter.
 *
 * @param key Key of intermediary element - K2.
 * @param value Value of intermediary element - V2.
 * @param context Contains data structure of the thread that created the intermediary element.
 */
void emit2 (K2* key, V2* value, void* context) {
    auto threadContext = (ThreadContext *) context;
    auto jobContext =  threadContext->getJobContext();
    threadContext->_intermediateVec->push_back({key, value});
    jobContext->atomicTotalPairsCount++; // These are the total amount of pairs in the shuffle and reduce stages

}

/**
 * Receives as input output element (K3, V3) and context which contains data
 * structure of the thread that created the output element.
 * Saves the output element in the context data structures (output vector).
 * In addition, updates the number of output elements using atomic counter.
 *
 * @param key Key of Output element - K3.
 * @param value Value of Output element - V3.
 * @param context Contains data structure of the thread that created the output element.
 */
void emit3 (K3* key, V3* value, void* context) {
    auto *jobContext = (JobContext *) context;
    lockMutex(jobContext->_mutex_saveOutput);
    jobContext->_outputVec->push_back({key, value});
    unlockMutex(jobContext->_mutex_saveOutput);
}
