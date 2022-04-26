#include "MapReduceFramework.h"

/**
 * This function starts running the MapReduce algorithm (with several threads)
 * and returns a JobHandle.
 *
 * @param client The task that the framework should run.
 * @param inputVec The input elements.
 * @param outputVec An empty vector to which the output elements
 * will be added before returning.
 * @param multiThreadLevel A valid number of worker threads to be used for running the algorithm.
 * @return JobHandle that will be used for monitoring the job.
 */
JobHandle startMapReduceJob (const MapReduceClient& client,
                             const InputVec& inputVec,
                             OutputVec& outputVec,
                             int multiThreadLevel) {

}

/**
 * Gets JobHandle returned by startMapReduceFramework and waits until it is finished.
 *
 * @param job A running job.
 */
void waitForJob(JobHandle job) {

}

/**
 * Gets a JobHandle and updates the state of the job into the given JobState struct.
 *
 * @param job A running job.
 * @param state State to update.
 */
void getJobState(JobHandle job, JobState* state) {

}

/**
 * Releasing all resources of a job.
 * Note: in case that the function is called and the job is not finished yet
 * wait until the job is finished to close it.
 *
 * @param job A running job.
 */
void closeJobHandle(JobHandle job) {

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

}
