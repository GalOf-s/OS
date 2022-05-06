#include <iostream>
#include "MapReduceFramework.h"
#include "ThreadContext.h"
#include "JobContext.h"

#define SYSTEM_ERROR "system error: "
#define MEMORY_ALLOCATION_ERROR "failed to allocate memory."


void systemError(const std::string &string) {
    std::cerr << SYSTEM_ERROR + string << std::endl;
    exit(EXIT_FAILURE);
}


/**
 * This function starts running the MapReduce algorithm (with several _threads)
 * and returns a JobHandle.
 *
 * @param client The task that the framework should run.
 * @param inputVec The input elements.
 * @param outputVec An empty vector to which the output elements
 * will be added before returning.
 * @param multiThreadLevel A valid number of worker _threads to be used for running the algorithm.
 * @return JobHandle that will be used for monitoring the job.
 */
JobHandle startMapReduceJob (const MapReduceClient& client,
                             const InputVec& inputVec,
                             OutputVec& outputVec,
                             int multiThreadLevel) {
	JobContext *jobContext;
    try{
        jobContext = static_cast<JobContext *>(new JobContext(multiThreadLevel,
															  &client,
															  &inputVec,
                                                              &outputVec));
    } catch (std::bad_alloc &) {
        systemError(MEMORY_ALLOCATION_ERROR);
    }
    return jobContext;
}

/**
 * Gets JobHandle returned by startMapReduceFramework and waits until it is finished.
 *
 * @param job A running job.
 */
void waitForJob(JobHandle job) {
    auto *jobContext = (JobContext *) job;
    if(jobContext->wasWaitForJobCalled()) {
        return;
    }
    jobContext->joinThreads();
}

/**
 * Gets a JobHandle and updates the state of the job into the given JobState struct.
 *
 * @param job A running job.
 * @param state State to update.
 */
void getJobState(JobHandle job, JobState* state) {
	auto jobContext = (JobContext *) job;
	jobContext->updateState();
	*state = jobContext->jobState;
}

/**
 * Releasing all resources of a job.
 * Note: in case that the function is called and the job is not finished yet
 * wait until the job is finished to close it.
 *
 * @param job A running job.
 */
void closeJobHandle(JobHandle job) {
    waitForJob(job);
    auto *jobContext = (JobContext *) job;
    delete jobContext;
}

/**
 * Receives as input intermediary element (K2, V2) and context which contains data
 * structure of the thread that created the intermediary element.
 * Saves the intermediary element in the context data structures.
 * In addition, updates the number of intermediary elements using atomic counter.
 *
 * @param key Key of intermediary element - K2.
 * @param value Value of intermediary element - V2.
 * @param context Contains data structure of the thread that created the intermediary element.
 */
void emit2 (K2* key, V2* value, void* context) {
    auto contexts = (JobContext::emit2Context *) context;
    IntermediatePair intermediatePair(key, value);
	contexts->threadContext->storeMapResult(intermediatePair);
	contexts->jobContext->atomicProgressTracker++;
}

/**
 * Receives as input output element (K3, V3) and context which contains data
 * structure of the thread that created the output element.
 * Saves the output element in the context data structures (output vector).
 * In addition, updates the number of output elements using atomic counter.
 *
 * @param key Key of Output element - K3.
 * @param value Value of Output element - V3.
 * @param context Contains data structure of the thread that created the output element.
 */
void emit3 (K3* key, V3* value, void* context) {
    auto *jobContext = (JobContext *) context;
    OutputPair outputPair(key, value);
    jobContext->storeReduceResult(outputPair);
    jobContext->atomicProgressTracker++;
	// TODO count total jobs?

}
