#ifndef EX3_MAPREDUCEWORKER_H
#define EX3_MAPREDUCEWORKER_H

#include "MapReduceFramework.h"
#include "MapReduceFramework.cpp"

#define PTHREAD_MUTEX_INIT_ERROR "pthread mutex init failed."
#define PTHREAD_MUTEX_DESTROY_ERROR "pthread mutex destroy failed."
#define PTHREAD_MUTEX_LOCK_ERROR "pthread mutex lock failed."
#define PTHREAD_MUTEX_UNLOCK_ERROR "pthread mutex unlock failed."

class MapReduceWorker
{
public:

    static void MapReduceWorker_init(const MapReduceClient& client, const InputVec& inputVec);
	explicit MapReduceWorker(int id);
	void *run(void *args);
	void storeMapResult(IntermediatePair intermediatePair){_intermediateVec.push_back(intermediatePair);}

private:
    static const MapReduceClient* s_mapReduceClient;
    static const InputVec* s_inputVec;
    static std::atomic<int> s_atomic_inputVectorIndex;
    static std::atomic<int> s_atomic_progressCounter;
    static JobState s_jobState;
    static Barrier* s_barrier;
    static pthread_mutex_t s_mutex_stagePercentage;
    static std::vector<IntermediateVec> shuffleVec;

	int _id;
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


#endif //EX3_MAPREDUCEWORKER_H
