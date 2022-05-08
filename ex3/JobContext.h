
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




#define SYSTEM_ERROR "system error: "
#define PTHREAD_CREATE_ERROR "pthread create failed."
#define MEMORY_ALLOCATION_ERROR "failed to allocate memory."
#define PTHREAD_MUTEX_INIT_ERROR "pthread mutex init failed."
#define PTHREAD_MUTEX_LOCK_ERROR "pthread mutex lock failed."
#define PTHREAD_MUTEX_UNLOCK_ERROR "pthread mutex unlock failed."
#define PTHREAD_MUTEX_DESTROY_ERROR "pthread mutex destroy failed."
#define SEM_INIT_ERROR "semaphore initialization failed."
#define SEM_DOWN_ERROR "semaphore down failed."
#define SEM_POST_ERROR "semaphore post failed."
#define SEM_DESTROY_ERROR "semaphore destroy failed."
#define PTHREAD_JOIN_ERROR "pthread join failed."
#define UNDEFINED_TOTAL_WORK -1



class JobContext {

public:
    typedef struct {
        ThreadContext* threadContext;
        JobContext* jobContext;
    } emit2Context;


    JobContext(int multiThreadLevel,
               const MapReduceClient *client,
               const InputVec *inputVec,
               OutputVec *outputVec);

    JobState jobState{};

    void storeReduceResult(OutputPair outputPair);
    void updateState();
    ~JobContext();

    bool wasWaitForJobCalled();

    void joinThreads();

	std::atomic<uint64_t> atomicProgressTracker{};
	std::atomic<unsigned long> atomicTotalPairsCount{};
private:
    int _multiThreadLevel;
    int _shuffleStageTotalWork;
    const MapReduceClient *_mapReduceClient;
    const InputVec *_inputVec;
    OutputVec *_outputVec;
    std::vector<ThreadContext> _threadContexts;
    std::vector<IntermediateVec> _shuffleVec;
    std::atomic<unsigned long> _atomic_nextIndex{};
	bool _isWaitForJobCalled;
    pthread_mutex_t _mutex_waitForJob{};
    pthread_mutex_t _mutex_saveOutput{};
    pthread_mutex_t _mutex_updateState{};
    sem_t _sem_reducePhase{};
    pthread_t *_threads{};
    Barrier *_barrier{};
    static void _systemError(const std::string &string);
    static void _lockMutex(pthread_mutex_t &mutex); // TODO check if needed more mutex, if not needed delete arg
    static void _unlockMutex(pthread_mutex_t &mutex);
    static void _destroyMutex(pthread_mutex_t &mutex);
    static bool _equalKeys(K2 *firstKey, K2 *secondKey);
    static unsigned long getCompletedCount(uint64_t progressTrackerValue);
    static unsigned long getTotalCount(uint64_t progressTrackerValue);
    static unsigned long getState(uint64_t progressTrackerValue);
     static unsigned long getNextIndex(u_int64_t progressTrackerValue);

    void _memoryAllocation();
    void _initSaveOutputMutex();
    void _initWaitForJobMutex();
    void _initReduceSem();
    void *_run(void *inputThreadContext);
    void _mapPhase(ThreadContext *threadContext);
//    void _incProgress();
    void _createThreads();
    void _shufflePhase();
    IntermediateVec _getMaxVec(K2 *maxKey);
    void _reducePhase();
    void _reduceSemDown();

    void _destroySem();

//    void _wakeUpThreads() const;
    void _wakeUpThreads(sem_t &sem) const;


    K2 *_getMaxKey();

    unsigned long _getTotalWork();

    unsigned long _calcShuffleStageTotalWork();

	void _initUpdateStateMutex();
};


#endif //EX3_JOBCONTEXT_H
