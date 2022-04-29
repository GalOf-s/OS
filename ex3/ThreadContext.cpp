#include <algorithm>
#include "ThreadContext.h"
#include "Barrier.h"


const MapReduceClient* ThreadContext::s_mapReduceClient;
const InputVec* ThreadContext::s_inputVec;
std::atomic<int> ThreadContext::s_atomic_inputVectorIndex;
std::atomic<int> ThreadContext::s_atomic_progressCounter;
JobState ThreadContext::s_jobState;
pthread_mutex_t ThreadContext::s_mutex_stagePercentage;
std::vector<IntermediateVec> ThreadContext::shuffleVec;

ThreadContext::ThreadContext(int id)
{
	_id = id;
}

void ThreadContext::ThreadContext_init(const MapReduceClient& client, const InputVec& inputVec)
{
    s_mapReduceClient = &client;
    s_inputVec = &inputVec;
    s_atomic_inputVectorIndex = 0;
    s_atomic_progressCounter = 0;
    s_jobState = {UNDEFINED_STAGE, 0};
    _initMutex(s_mutex_stagePercentage);
}

void *ThreadContext::run()
{
    s_jobState.stage = MAP_STAGE; // TODO check if is it ok that all threads do this?
	mapPhase();
	sortPhase();
    s_barrier->barrier();
    if(_id == 0) {
        shufflePhase();
    }
    reducePhase();
}

int ThreadContext::mapPhase()
{
	int inputVectorIndex = (s_atomic_inputVectorIndex)++;
	while(inputVectorIndex < s_inputVec->size()){
        InputPair nextPair = s_inputVec->at(inputVectorIndex);
        s_mapReduceClient->map(nextPair.first, nextPair.second, this);
        (s_atomic_progressCounter)++;

        _lockMutex(s_mutex_stagePercentage);
        s_jobState.percentage = ((float) s_atomic_progressCounter / (float) s_inputVec->size()) * 100;
        _unlockMutex(s_mutex_stagePercentage);

        inputVectorIndex = (s_atomic_inputVectorIndex)++;
	}
	return 0;
}

bool pairComparer(IntermediatePair pair1, IntermediatePair pair2){
	return pair1.first < pair2.first;
}

int ThreadContext::sortPhase()
{
	std::sort(this->_intermediateVec.begin(), this->_intermediateVec.end(), pairComparer);

	return 0;
}

int ThreadContext::shufflePhase() {
    return 0;
}

void ThreadContext::_initMutex(pthread_mutex_t &mutex) {
    if (pthread_mutex_init(&mutex, nullptr) != 0) {
        _systemError(PTHREAD_MUTEX_INIT_ERROR);
    }
}

void ThreadContext::_lockMutex(pthread_mutex_t &mutex) {
    if (pthread_mutex_lock(&mutex) != 0) {
        _systemError(PTHREAD_MUTEX_LOCK_ERROR);
    }
}

void ThreadContext::_unlockMutex(pthread_mutex_t &mutex) {
    if (pthread_mutex_unlock(&mutex) != 0) {
        _systemError(PTHREAD_MUTEX_UNLOCK_ERROR);
    }
}


void ThreadContext::_systemError(const std::string &string) {
    std::cerr << SYSTEM_ERROR + string << std::endl;
    exit(EXIT_FAILURE);
}


