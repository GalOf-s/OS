#ifndef EX3_THREADCONTEXT_H
#define EX3_THREADCONTEXT_H

#include "MapReduceFramework.h"
#include "MapReduceFramework.cpp"

#define PTHREAD_MUTEX_INIT_ERROR "pthread mutex init failed."
#define PTHREAD_MUTEX_DESTROY_ERROR "pthread mutex destroy failed."
#define PTHREAD_MUTEX_LOCK_ERROR "pthread mutex lock failed."
#define PTHREAD_MUTEX_UNLOCK_ERROR "pthread mutex unlock failed."

class ThreadContext
{
public:
	explicit ThreadContext(int id, JobContext *jobContext);
	void *run();
	void storeMapResult(IntermediatePair intermediatePair){_intermediateVec.push_back(intermediatePair);}

private:
	int _id;
	JobContext* _jobContext;
    IntermediateVec _intermediateVec;


    static void _initMutex(pthread_mutex_t &mutex);
    static void _lockMutex(pthread_mutex_t &mutex);
    static void _unlockMutex(pthread_mutex_t &mutex);
    static void _systemError(const std::string &string);
    int mapPhase();
    int sortPhase();
    int shufflePhase();
    int reducePhase();
	// int _progressCount;
};


#endif //EX3_THREADCONTEXT_H
