#ifndef EX3_MAPREDUCEWORKER_H
#define EX3_MAPREDUCEWORKER_H

#include "MapReduceFramework.h"
#include "MapReduceFramework.cpp"

class MapReduceWorker
{
public:
	explicit MapReduceWorker(int id, JobContext* jobContext, const MapReduceClient* client);
	void run(const InputVec& inputVec, OutputVec& outputVec);
	void storeMapResult(IntermediatePair intermediatePair){this->intermediateVec.push_back
	(intermediatePair);}

private:
	int _id;
	JobContext* _jobContext;
	const MapReduceClient* _mapReduceClient;
	int mapPhase(const InputVec& inputVec);
	int sortPhase();
	int shufflePhase(const InputVec& inputVec);
	int reducePhase(const InputVec& inputVec);
	IntermediateVec intermediateVec;
	int _progressCount;
};


#endif //EX3_MAPREDUCEWORKER_H
