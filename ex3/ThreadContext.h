#ifndef EX3_THREADCONTEXT_H
#define EX3_THREADCONTEXT_H

#include "MapReduceFramework.h"
#include "JobContext.h"
#include <iostream>

class JobContext;

class ThreadContext
{
public:

    ThreadContext(int id, JobContext *jobContext);

    JobContext *getJobContext();
    void sortPhase() const;
    void storeMapResult(IntermediatePair intermediatePair);
//    static void _initSaveOutputMutex(pthread_mutex_t &mutex);
//    static void _lockMutex(pthread_mutex_t &mutex);
//    static void _unlockMutex(pthread_mutex_t &mutex);
//    static void _systemError(const std::string &string);

    IntermediateVec *_intermediateVec;

    IntermediatePair getMaxPair() const;
    K2* getMaxKey() const;

    bool isIntermediateVecEmpty() const;
    void deleteIntermediateVec() const;

    int getId() const;

//    IntermediateVec *_intermediateVec;
//    int mapPhase();
//    int shufflePhase();
//    int reducePhase();
	// int _progressCount;

private:
    int _id;
    JobContext *_jobContext;

};


#endif //EX3_THREADCONTEXT_H
