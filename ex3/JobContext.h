
#ifndef EX3_JOBCONTEXT_H
#define EX3_JOBCONTEXT_H


#include "ThreadContext.h"
#include "Barrier.h"
#include <atomic>
#include <semaphore.h>
#include <pthread.h>
#include <iostream>


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


class JobContext {

public:

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

private:
    int _multiThreadLevel;
    const MapReduceClient *_mapReduceClient;
    const InputVec *_inputVec;
    OutputVec *_outputVec;
    std::vector<ThreadContext> _threadContexts;
    std::vector<IntermediateVec> _shuffleVec;
    std::atomic<int> _atomic_nextIndex{};
    std::atomic<uint64_t> _atomic_progressTracker{};
    bool _isWaitForJobCalled;
    pthread_mutex_t _mutex_waitForJob{};
    pthread_mutex_t _mutex_saveOutput{};
    sem_t *_sem_reducePhase{};
    pthread_t *_threads{};
    Barrier *_barrier{};
    static void _systemError(const std::string &string);
    static void _lockMutex(pthread_mutex_t &mutex); // TODO check if needed more mutex, if not needed delete arg
    static void _unlockMutex(pthread_mutex_t &mutex);
    static void _destroyMutex(pthread_mutex_t &mutex);
    static bool _equalKeys(K2 *firstKey, K2 *secondKey);
    static unsigned long getCompletedCount(uint64_t progressTrackerValue);
//    static unsigned long getTotalCount(uint64_t progressTrackerValue);
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

    void _wakeUpThreads() const;


    K2 *_getMaxKey();
};


#endif //EX3_JOBCONTEXT_H
