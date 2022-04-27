#include <algorithm>
#include "MapReduceWorker.h"
#include "Barrier.h"

MapReduceWorker::MapReduceWorker(int id, JobContext* jobContext, const MapReduceClient* client)
{
	_id = id;
	_jobContext = jobContext;
	_mapReduceClient = client;
}

void MapReduceWorker::run(const InputVec &inputVec, OutputVec &outputVec)
{
	mapPhase(inputVec);
	sortPhase();
}

int MapReduceWorker::mapPhase(const InputVec &inputVec)
{
	_progressCount = 0;
	InputPair next_pair;
	int next_pair_index = (*(this->_jobContext->input_vector_index))++;
	while(next_pair_index < inputVec.size()){
		next_pair = inputVec[next_pair_index];
		this->_mapReduceClient->map(next_pair.first, next_pair.second,this);
		_progressCount++;
		next_pair_index = (*(this->_jobContext->input_vector_index))++;
	}
	return 0;
}

bool pairComparer(IntermediatePair pair1, IntermediatePair pair2){
	return pair1.first < pair2.first;
}

int MapReduceWorker::sortPhase()
{
	std::sort(this->intermediateVec.begin(), this->intermediateVec.end(), pairComparer);
	this->_jobContext->barrier->barrier();
	return 0;
}
