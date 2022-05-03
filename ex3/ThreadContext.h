#ifndef EX3_THREADCONTEXT_H
#define EX3_THREADCONTEXT_H

#include "MapReduceFramework.h"
#include "MapReduceFramework.cpp"

class ThreadContext
{
public:
    explicit ThreadContext(int id);
    int getId() const;
    void sortPhase();
    void storeMapResult(IntermediatePair intermediatePair){_intermediateVec.push_back(intermediatePair);}
//    static void _initSaveOutputMutex(pthread_mutex_t &mutex);
//    static void _lockMutex(pthread_mutex_t &mutex);
//    static void _unlockMutex(pthread_mutex_t &mutex);
//    static void _systemError(const std::string &string);

    IntermediatePair getMaxPair();
    K2* getMaxKey();

    bool isIntermediateVecEmpty();

private:
    int _id;
    IntermediateVec _intermediateVec;

//    int mapPhase();
//    int shufflePhase();
//    int reducePhase();
	// int _progressCount;
};


#endif //EX3_THREADCONTEXT_H
