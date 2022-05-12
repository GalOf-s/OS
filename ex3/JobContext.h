#ifndef EX3_JOBCONTEXT_H
#define EX3_JOBCONTEXT_H


#include "ThreadContext.h"
#include "Barrier.h"
#include <atomic>
#include <semaphore.h>
#include <pthread.h>
#include <vector>
#include <iostream>
//
//#define NEXT_INDEX_OFFSET 31
//#define STAGE_OFFSET 62
//
//#define COMPLETED_COUNT_MASK 0x7fffffff // first 31 bit store the number of already processed keys
//#define NEXT_INDEX_MASK 0x3fffffff80000000 // second 31 bit store the next index to process in the input vector
//#define STAGE_MASK 0xC000000000000000 // last  2 bits to flag the stage


#define TOTAL_COUNT_OFFSET 31
#define STAGE_OFFSET 62

#define COMPLETED_COUNT_MASK 0x7fffffff // first 31 bit store the number of already processed keys
#define TOTAL_COUNT_MASK 0x3fffffff80000000 // second 31 bit store number of total keys to process
#define STAGE_MASK 0xC000000000000000 // last  2 bits to flag the stage




#define SYSTEM_ERROR_MSG "system error: "
#define PTHREAD_CREATE_ERROR_MSG "pthread create failed."
#define MEMORY_ALLOCATION_ERROR_MSG "failed to allocate memory."
#define PTHREAD_MUTEX_INIT_ERROR_MSG "pthread mutex init failed."
#define PTHREAD_MUTEX_LOCK_ERROR_MSG "pthread mutex lock failed."
#define PTHREAD_MUTEX_UNLOCK_ERRO_MSG "pthread mutex unlock failed."
#define PTHREAD_MUTEX_DESTROY_ERROR_MSG "pthread mutex destroy failed."
#define PTHREAD_JOIN_ERROR "pthread join failed."
#define ROUND_FACTOR_PERCENTAGE 100.0


class ThreadContext;

class JobContext {

public:

    /**
     * Constructor
     */
    JobContext(int multiThreadNum,
                           const MapReduceClient *client,
                           const InputVec *inputVector,
                           OutputVec *outputVector);

    /**
     * Gets the maximum key out of all the threads intermediates vectors.
     */
    K2 *getMaxKey() const;

    /**
     * Makes vector with all elements according to maxKey
     */
    void makeMaxVec(IntermediateVec *maxVec, K2 *maxKey) const;

    /**
     * Gets key and a value and stores it as output pair in the output vector
     */
    void storeReduceResult(K3 *key, V3 *value);

    /**
     * Updates the state of this job.
     */
    void updateState();

    /**
     * Handles a call of wait for job
     */
    bool wasWaitForJobCalled();

    /**
     * Joins all thread.
     */
    void joinThreads() const;

    /**
     * Destructor.
     */
    ~JobContext();

    int multiThreadLevel;
    const MapReduceClient *mapReduceClient;
    const InputVec *inputVec;
    OutputVec *outputVec;
    Barrier *barrier{};
    pthread_t *threads{};
    JobState jobState{};
    std::vector<ThreadContext *> *threadContexts{};
    std::vector<IntermediateVec*> *shuffleVec{};
    std::atomic<uint64_t> atomicProgressTracker{};
    std::atomic<unsigned long> atomicTotalPairsCount{};
    std::atomic<unsigned long> atomic_nextIndex{};
    bool _isWaitForJobCalled;
    pthread_mutex_t _mutex_waitForJob{};
    pthread_mutex_t mutex_saveOutput{};
    pthread_mutex_t mutex_updateState{};


private:

    /**
     * Returns the value of the second 31 bit of the atomicProgressTracker,
     * that is the number of total keys to process
     */
    static unsigned long _getTotalCount(uint64_t progressTrackerValue);

    /**
    * Returns the value of the 2 last bit bit of the atomicProgressTracker,
     * that is the job current state
    */
    static unsigned long _getState(uint64_t progressTrackerValue);

    /**
     * Returns the value of the first 31 bit of the atomicProgressTracker,
     * the number of already processed keys.
     */
    static unsigned long _getCompletedCount(uint64_t progressTrackerValue);

    /**
     * locks mutex.
     */
    static void _lockMutex(pthread_mutex_t &mutex);

    /**
     * unlocks mutex.
     */
    static void _unlockMutex(pthread_mutex_t &mutex);

    /**
     * destroy mutex.
     */
    static void _destroyMutex(pthread_mutex_t &mutex);

    /**
     * prints system error to stderr
     */
    static void _systemError(const std::string &string);

    /**
     * Return true if two keys are equal, and faults otherwise
     */
    static bool _equalKeys(K2 *firstKey, K2 *secondKey);

    /**
     * allocates object on the heap.
     */
    void _memoryAllocation();

    /**
     * init for saveOutput mutex
     */
    void _initSaveOutputMutex();

    /**
     * init for waitForJob mutex
     */
    void _initWaitForJobMutex();

    /**
     * init for updateStage mutex
     */
    void _initUpdateStateMutex();

};


#endif //EX3_JOBCONTEXT_H