#include <algorithm>
#include "ThreadContext.h"
#include "Barrier.h"



ThreadContext::ThreadContext(int id, JobContext* jobContext)
{
	_id = id;
	_jobContext = jobContext;
}

void *ThreadContext::run()
{
	_jobContext->jobState.stage = MAP_STAGE; // TODO check if is it ok that all threads do this?
	mapPhase();
	sortPhase();
	_jobContext->barrier->barrier(); // TODO: is it right to init here?
    if(_id == 0) {
        shufflePhase();
    }
    reducePhase();
}

int ThreadContext::mapPhase()
{
	int inputVectorIndex = (_jobContext->atomic_inputVectorIndex)++;
	while(inputVectorIndex < _jobContext->inputVec->size()){
        InputPair nextPair = _jobContext->inputVec->at(inputVectorIndex);
        _jobContext->mapReduceClient->map(nextPair.first, nextPair.second, this);
        (_jobContext->atomic_progressCounter)++;

        _lockMutex(_jobContext->s_mutex_stagePercentage);
		_jobContext->jobState.percentage = ((float) _jobContext->atomic_progressCounter / (float) _jobContext->inputVec->size())
										   * 100;
        _unlockMutex(_jobContext->s_mutex_stagePercentage);

        inputVectorIndex = (_jobContext->atomic_inputVectorIndex)++;
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


