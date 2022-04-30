#ifndef EX3_JOBCONTEXT_H
#define EX3_JOBCONTEXT_H


#include <string>
#include "ThreadContext.h"

#define SYSTEM_ERROR "system error: "
#define PTHREAD_CREATE_ERROR "pthread create failed."
#define MEMORY_ALLOCATION_ERROR "failed to allocate memory."


class JobContext {

public:
    explicit JobContext(int multiThreadLevel, const MapReduceClient* client, const
	InputVec* vec);
	const MapReduceClient* mapReduceClient{};
	const InputVec* inputVec{};
	std::atomic<int> atomic_inputVectorIndex{};
	std::atomic<int> atomic_progressCounter{};
	JobState jobState{};
	Barrier* barrier{};
	pthread_mutex_t s_mutex_stagePercentage{};
	std::vector<IntermediateVec> shuffleVec;
	int incProgress();

private:
    int _multiThreadLevel;
    std::vector<ThreadContext> threadContexts;
    pthread_t *threads;
    static void _systemError(const std::string &string);
    void _createThreads();

};


#endif //EX3_JOBCONTEXT_H
