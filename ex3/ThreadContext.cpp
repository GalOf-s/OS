#include <algorithm>
#include "ThreadContext.h"


ThreadContext::ThreadContext(int id, JobContext *jobContext)
{
    _id = id;
    _jobContext = jobContext;
    _intermediateVec = new IntermediateVec();
}

JobContext *ThreadContext::getJobContext() const{
    return _jobContext;
}

void ThreadContext::deleteIntermediateVec() const
{
    delete _intermediateVec;
}

IntermediatePair ThreadContext::getMaxPair() const {
    IntermediatePair currentMaxPair = _intermediateVec->back();
    _intermediateVec->pop_back();
    return currentMaxPair;
}

K2* ThreadContext::getMaxKey() const{
    return _intermediateVec->back().first;
}

bool ThreadContext::isIntermediateVecEmpty() const {
    return _intermediateVec->empty();
}

int ThreadContext::getId() const {
    return _id;
}

void ThreadContext::storeMapResult(IntermediatePair intermediatePair) const {
    _intermediateVec->push_back(intermediatePair);
}

IntermediateVec *ThreadContext::getIntermediateVec() {
    return _intermediateVec;
}

ThreadContext::~ThreadContext(){
    delete _intermediateVec;
}