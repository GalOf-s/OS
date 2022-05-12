#include <iostream>
#include "MapReduceFramework.h"
#include "ThreadContext.h"
#include "JobContext.h"
#include <pthread.h>
#include <algorithm>


/**
 * prints system error to stderr
 */
void systemError(const std::string &string) {
    std::cerr << SYSTEM_ERROR_MSG + string << std::endl;
    exit(EXIT_FAILURE);
}

/**
 * locks mutex
 */
void lockMutex(pthread_mutex_t &mutex) {
    if (pthread_mutex_lock(&mutex) != 0) {
        systemError(PTHREAD_MUTEX_LOCK_ERROR_MSG);
    }
}

/**
 * unlocks mutex
 */
void unlockMutex(pthread_mutex_t &mutex) {
    if (pthread_mutex_unlock(&mutex) != 0) {
        systemError(PTHREAD_MUTEX_UNLOCK_ERRO_MSG);
    }
}

/**
 * Handles the map phase function for one thread
 */
void mapPhase(ThreadContext *threadContext) {
    auto jobContext = threadContext->getJobContext();
    lockMutex(jobContext->mutex_updateState);
    if (jobContext->atomicProgressTracker.load() < (((uint64_t) MAP_STAGE) << STAGE_OFFSET)) {
        jobContext->atomicProgressTracker = ((uint64_t) MAP_STAGE << STAGE_OFFSET);
    }
    unlockMutex(jobContext->mutex_updateState);

    unsigned long inputVectorIndex = (jobContext->atomic_nextIndex)++;
    while (inputVectorIndex < jobContext->inputVec->size()) {
        InputPair nextPair = jobContext->inputVec->at(inputVectorIndex);
        jobContext->mapReduceClient->map(nextPair.first, nextPair.second, threadContext);
        inputVectorIndex = (jobContext->atomic_nextIndex)++;
        jobContext->atomicProgressTracker++; // update count of completed input pairs
    }
}

/**
 * Handles the shuffle phrase.
 * Note: only thread 0 runs this function.
 */
void shufflePhase(JobContext *jobContext) {
    K2 *maxKey = jobContext->getMaxKey();
    while (maxKey) {
        auto *maxVec = new IntermediateVec();
        jobContext->makeMaxVec(maxVec, maxKey);
        jobContext->shuffleVec->push_back(maxVec);
        jobContext->atomicProgressTracker += maxVec->size();
        maxKey = jobContext->getMaxKey();
    }
}

/**
 * Handles the reduce phrase function for one thread
 */
void reducePhase(JobContext *jobContext) {
    unsigned long shuffleVectorIndex = jobContext->atomic_nextIndex++;

    while (shuffleVectorIndex < jobContext->shuffleVec->size()) {
        IntermediateVec *nextVec = jobContext->shuffleVec->at(shuffleVectorIndex);
        jobContext->mapReduceClient->reduce(nextVec, jobContext);
        jobContext->atomicProgressTracker += nextVec->size();
        shuffleVectorIndex = jobContext->atomic_nextIndex++;
        delete nextVec;
    }
}


/**
 * Pair comparer function for sorting algorithem
 */
bool pairComparer(IntermediatePair pair1, IntermediatePair pair2) {
    return *pair1.first < *pair2.first;
}

/**
 * Handle the entire MapReduce job.
 * This is the function that all threads run.
 */
void *run(void *args) {
    auto threadContext = (ThreadContext *) args;
    auto jobContext = threadContext->getJobContext();
    mapPhase(threadContext);

    IntermediateVec *threadVec = threadContext->getIntermediateVec();
    std::sort(threadVec->begin(), threadVec->end(), pairComparer);

    jobContext->barrier->barrier();

    if (threadContext->getId() == 0) {
        jobContext->atomicProgressTracker = (((uint64_t) SHUFFLE_STAGE) << STAGE_OFFSET) +
                                            (jobContext->atomicTotalPairsCount << TOTAL_COUNT_OFFSET);
        shufflePhase(jobContext);

        jobContext->atomicProgressTracker =
                (((uint64_t) REDUCE_STAGE) << STAGE_OFFSET) + (jobContext->atomicTotalPairsCount << TOTAL_COUNT_OFFSET);
        jobContext->atomic_nextIndex = 0;
        jobContext->atomicTotalPairsCount = 0;
    }

    jobContext->barrier->barrier();

    reducePhase(jobContext);
    return nullptr;
}

/**
 * Creates all threads.
 */
void createThreads(JobContext *jobContext) {
    jobContext->threadContexts->reserve(jobContext->multiThreadLevel);
    for (int i = 0; i < jobContext->multiThreadLevel; i++) {
        jobContext->threadContexts->emplace_back(new ThreadContext(i, jobContext));
        if (pthread_create(jobContext->threads + i, nullptr, run, jobContext->threadContexts->at(i)) != 0) {
            systemError(PTHREAD_CREATE_ERROR_MSG);
        }
    }
}

/**
 * This function starts running the MapReduce algorithm (with several threads)
 * and returns a JobHandle.
 *
 * @param client The task that the framework should run.
 * @param inputVec The input elements.
 * @param outputVec An empty vector to which the output elements
 * will be added before returning.
 * @param multiThreadLevel A valid number of worker threads to be used for running the algorithm.
 * @return JobHandle that will be used for monitoring the job.
 */
JobHandle startMapReduceJob(const MapReduceClient &client,
                            const InputVec &inputVec,
                            OutputVec &outputVec,
                            int multiThreadLevel) {
    JobContext *jobContext;
    try {
        jobContext = static_cast<JobContext *>(new JobContext(multiThreadLevel,
                                                              &client,
                                                              &inputVec,
                                                              &outputVec));
        createThreads(jobContext);
    } catch (std::bad_alloc &) {
        systemError(MEMORY_ALLOCATION_ERROR_MSG);
    }
    return jobContext;
}

/**
 * Gets JobHandle returned by startMapReduceFramework and waits until it is fini
 * shed.
 *
 * @param job A running job.
 */
void waitForJob(JobHandle job) {
    auto *jobContext = (JobContext *) job;
    if (jobContext->wasWaitForJobCalled()) {
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
void getJobState(JobHandle job, JobState *state) {
    auto jobContext = (JobContext *) job;
    jobContext->updateState();
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
void emit2(K2 *key, V2 *value, void *context) {
    auto threadContext = (ThreadContext *) context;
    auto jobContext = threadContext->getJobContext();
    IntermediatePair intermediatePair(key, value);
    threadContext->storeMapResult(intermediatePair);
    jobContext->atomicTotalPairsCount++;

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
void emit3(K3 *key, V3 *value, void *context) {
    auto *jobContext = (JobContext *) context;
    jobContext->storeReduceResult(key, value);
}