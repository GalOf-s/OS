#include "JobContext.h"

JobContext::JobContext(int multiThreadLevel) {
    try{
        threads = new pthread_t[_multiThreadLevel];
    } catch (std::bad_alloc &) {
        _systemError(MEMORY_ALLOCATION_ERROR);
    }
    _multiThreadLevel = multiThreadLevel;
    threadContexts = std::vector<ThreadContext>(multiThreadLevel);
    _createMultiThreadLevel();
}

void JobContext::_createMultiThreadLevel() {
    for (int i = 0; i < _multiThreadLevel; i++) {
        threadContexts[i] = ThreadContext(i);
        if (pthread_create(threads + i, nullptr,
                           [](void *obj){ return ((ThreadContext *)obj)->run(); },
                           &threadContexts[i]) != 0) {
            _systemError(PTHREAD_CREATE_ERROR);

        }
    }

}

void JobContext::_systemError(const std::string &string) {
    std::cerr << SYSTEM_ERROR + string << std::endl;
    exit(EXIT_FAILURE);
}

