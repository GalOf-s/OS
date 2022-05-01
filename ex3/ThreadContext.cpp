#include <algorithm>
#include "ThreadContext.h"


ThreadContext::ThreadContext(int id)
{
	_id = id;
}

bool pairComparer(IntermediatePair pair1, IntermediatePair pair2){
    return pair1.first < pair2.first;
}

void ThreadContext::sortPhase()
{
    std::sort(this->_intermediateVec.begin(), this->_intermediateVec.end(), pairComparer);
}

int ThreadContext::getId() const {
    return _id;
}

IntermediatePair ThreadContext::getMaxPair() {
    IntermediatePair currentMaxPair = _intermediateVec.back();
    _intermediateVec.pop_back();
    return currentMaxPair;
}

K2* ThreadContext::getMaxKey(){
    return _intermediateVec.back().first;
}

bool ThreadContext::isIntermediateVecEmpty() {
    return _intermediateVec.empty();
}


//void *ThreadContext::run()
//{
//	_jobContext->jobState.stage = MAP_STAGE; // TODO check if is it ok that all _threads do this?
//	mapPhase();
//	sortPhase();
//	_jobContext->_barrier->_barrier(); // TODO: is it right to init here?
//    if(_id == 0) {
//        shufflePhase();
//    }
//    reducePhase();
//}
//
//int ThreadContext::mapPhase()
//{
//	int inputVectorIndex = (_jobContext->_atomic_inputVectorIndex)++;
//	while(inputVectorIndex < _jobContext->_inputVec->size()){
//        InputPair nextPair = _jobContext->_inputVec->at(inputVectorIndex);
//        _jobContext->_mapReduceClient->map(nextPair.first, nextPair.second, this);
//        (_jobContext->_atomic_progressCounter)++;
//
//        _lockMutex(_jobContext->s_mutex_stagePercentage);
//		_jobContext->jobState.percentage = ((float) _jobContext->_atomic_progressCounter / (float) _jobContext->_inputVec->size())
//										   * 100;
//        _unlockMutex(_jobContext->s_mutex_stagePercentage);
//
//        inputVectorIndex = (_jobContext->_atomic_inputVectorIndex)++;
//	}
//	return 0;
//}


//int ThreadContext::shufflePhase() {
//    return 0;
//}

//void ThreadContext::_initMutex(pthread_mutex_t &mutex) {
//    if (pthread_mutex_init(&mutex, nullptr) != 0) {
//        _systemError(PTHREAD_MUTEX_INIT_ERROR);
//    }
//}
//
//void ThreadContext::_lockMutex(pthread_mutex_t &mutex) {
//    if (pthread_mutex_lock(&mutex) != 0) {
//        _systemError(PTHREAD_MUTEX_LOCK_ERROR);
//    }
//}
//
//void ThreadContext::_unlockMutex(pthread_mutex_t &mutex) {
//    if (pthread_mutex_unlock(&mutex) != 0) {
//        _systemError(PTHREAD_MUTEX_UNLOCK_ERROR);
//    }
//}
//
//
//void ThreadContext::_systemError(const std::string &string) {
//    std::cerr << SYSTEM_ERROR + string << std::endl;
//    exit(EXIT_FAILURE);
//}
//
//
