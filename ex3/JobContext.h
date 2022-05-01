
#ifndef EX3_JOBCONTEXT_H
#define EX3_JOBCONTEXT_H


#include "ThreadContext.h"

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



class JobContext {

public:

    JobContext(int multiThreadLevel,
               const MapReduceClient *client,
               const InputVec *inputVec,
               OutputVec *outputVec);

    void storeReduceResult(OutputPair outputPair);

private:
    int _multiThreadLevel;
    const MapReduceClient *_mapReduceClient;
    const InputVec *_inputVec;
    OutputVec *_outputVec;
    std::vector<ThreadContext> _threadContexts;
    std::vector<IntermediateVec> _shuffleVec;
    std::atomic<int> _atomic_inputVectorIndex{};
    std::atomic<int> _atomic_progressCounter{};
    std::atomic<int> _atomic_shuffleVectorSize{};
    pthread_mutex_t _mutex_stagePercentage{};
    sem_t _sem_reducePhase{};
    pthread_t *_threads{};
    Barrier *_barrier{};
    JobState _jobState{};

    static void _systemError(const std::string &string);
    static void _lockMutex(pthread_mutex_t &mutex); // TODO check if needed more mutex, if not needed delete arg
    static void _unlockMutex(pthread_mutex_t &mutex);
    static bool _equalKeys(K2 *firstKey, K2 *secondKey);


    void _memoryAllocation();
    void _initMutex();
    void _initSem();
    void *_run(void *args);
    void _mapPhase(ThreadContext *threadContext);
    void _incProgress();
    void _createThreads();
    void _shufflePhase();
    IntermediateVec _getMaxVec(K2 *maxKey);
    void _reducePhase();
    void _reduceSemDown();

    void _wakeUpThreads() const;

    K2 *_getMaxKey();

};


#endif //EX3_JOBCONTEXT_H
