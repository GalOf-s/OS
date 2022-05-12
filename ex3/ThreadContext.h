#ifndef EX3_THREADCONTEXT_H
#define EX3_THREADCONTEXT_H

#include "MapReduceFramework.h"
#include "JobContext.h"
#include <iostream>

class JobContext;

class ThreadContext
{
public:

    /**
     * Constructor.
     */
    ThreadContext(int id, JobContext *jobContext);

    /**
   * Gets for thread id.
   */
    int getId() const;

    /**
     * Getter for jobContext object.
     */
    JobContext *getJobContext() const;

    /**
    * Getter for the intermediate vector.
    */
    IntermediateVec *getIntermediateVec();

    /**
     * Returns the pair with the maximum key.
     */
    IntermediatePair getMaxPair() const;

    /**
     * Returns the maximum key.
     */
    K2* getMaxKey() const;

    /**
     * Returns True if the intermediate vector is empty and false otherwise.
     */
    bool isIntermediateVecEmpty() const;

    /**
     * Gets an intermediate pair object and stores it the the intermediate vector.
     */
    void  storeMapResult(IntermediatePair intermediatePair) const;

    /**
     * Destructor.
     */
    void ~ThreadContext();

private:

    int _id;
    IntermediateVec *_intermediateVec;
    JobContext *_jobContext;

};


#endif //EX3_THREADCONTEXT_H
