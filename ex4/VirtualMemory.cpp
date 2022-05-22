#include <algorithm>
#include "VirtualMemory.h"
#include "MemoryConstants.h"
#include "PhysicalMemory.h"


#define EMPTY_CELL_VALUE 0
#define FAILURE -1

#define SUCCESS 0

typedef struct{
    int frameIndex;
    int frameOffset;
} FrameAddress;

int readByIndex(int frameIndex, uint64_t frameOffset);

void findFrameToEvict(int startFrameIndex,
                      int currDepth,
                      int swapInPageIndex,
                      long long *currentMaxDistance,
                      FrameAddress *parentFrame,
                      FrameAddress *frameToEvict);

bool isFrameEmpty(int frameIndex);

uint64_t convertIndexesToAddress(uint64_t frameIndex, uint64_t frameOffset);

uint64_t translateToPhysicalAddress(uint64_t virtualAddress);

void translateToVirtualOffsets(uint64_t pageTable[], uint64_t pages);

void writeByIndex(int frameIndex, uint64_t frameOffset, word_t value);


word_t findNextFrame(word_t frame);

word_t firstTranslation(uint64_t pagesTable[]);

word_t secondTranslation(uint64_t pageNum, word_t lastFrame);

long long calcRowCyclicDistance(int frameIndex, int swapInPageIndex, int rowIndex);

long long calcFrameCyclicDistance(int frameIndex, int swapInPageIndex);

/** Finds the first frame in memory that satisfies the condition provided as a boolean function, by
 * preforming a DFS run on the page tree.
 *
 * returns the free frame's index upon success.
 * returns -1 if no empty frame was found.
 */
int findEmptyFrame(int startFrameIndex,
                   int currDepth,
                   int* maxFrameVisited,
                   FrameAddress *parentFrame,
                   bool (*condition)(int));
/**
 * Initializes a new frame according to the given frameIndex.
 */
void initNewFrame(int frameIndex) {
    for(int i = 0; i < PAGE_SIZE; i++) {
        writeByIndex(frameIndex, i, 0);
    }
}

/**
 * Initialize the virtual memory.
 */
void VMinitialize(){
    for(int i=0; i<NUM_FRAMES; i++){
        initNewFrame(i);
    }
}

/** Reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t* value) {
    // TODO if the address cannot be mapped to a physical address for any reason
    uint64_t physicalAddress = translateToPhysicalAddress(virtualAddress);
    PMread(physicalAddress, value);
    return SUCCESS;
}

/** Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite(uint64_t virtualAddress, word_t value){
    // TODO if the address cannot be mapped to a physic address for any reason
    uint64_t physicalAddress = translateToPhysicalAddress(virtualAddress);
    PMwrite(physicalAddress, value);
    return SUCCESS;
}


void translateToVirtualOffsets(uint64_t pageTable[], uint64_t pages) {
    for (int i = TABLES_DEPTH - 1; i >= 0; --i) {
        pageTable[i] = pages % PAGE_SIZE; // TODO check if this is the right macro to get the pages: [p1, p2]
        pages = pages >> PAGE_SIZE;
    }
}


/**
 * Return the translation of a virtual address to a physical address.
 */
uint64_t translateToPhysicalAddress(uint64_t virtualAddress) {
    uint64_t offset = virtualAddress % PAGE_SIZE;
    uint64_t pageNum = virtualAddress >> OFFSET_WIDTH;

    // an array to find the path from the root to the wanted page
    uint64_t pagesTable[TABLES_DEPTH]; // TODO check if this is the right size

    translateToVirtualOffsets(pagesTable, pageNum);

    word_t lastFrame = firstTranslation(pagesTable);
    word_t wantedPageFrame = secondTranslation(pageNum, lastFrame);

    return convertIndexesToAddress(wantedPageFrame, offset);
}

word_t secondTranslation(uint64_t pageNum, word_t lastFrame) {
    word_t lastFrameOffset = TABLES_DEPTH - 1;

    word_t wantedPageFrame = readByIndex(lastFrame, lastFrameOffset); // addr2
    if (wantedPageFrame == EMPTY_CELL_VALUE) {
        wantedPageFrame = findNextFrame(lastFrame);
        PMrestore(lastFrame, pageNum);
        writeByIndex(lastFrame, lastFrameOffset, wantedPageFrame);
    }
    return wantedPageFrame;
}

word_t firstTranslation(uint64_t pagesTable[]) {
    word_t currentFrame = 0; //addr1

    int i = 0;
    while (i < TABLES_DEPTH - 1) {
        word_t nextFrame = readByIndex(currentFrame, pagesTable[i]);
        if(nextFrame == EMPTY_CELL_VALUE) {
            nextFrame = findNextFrame(currentFrame);
            initNewFrame(nextFrame);
            writeByIndex(currentFrame, pagesTable[i], nextFrame);
        }
        currentFrame = nextFrame;
        i++;
    }
    return currentFrame;
}

word_t findNextFrame(word_t frame) {
    int maxFrameVisited = 0;
    FrameAddress parentFrame;
    int emptyFrameIndex = findEmptyFrame(frame,
                                         0,
                                         &maxFrameVisited,
                                         &parentFrame,
                                         isFrameEmpty);
    if (emptyFrameIndex != FAILURE){
        writeByIndex(parentFrame.frameIndex, parentFrame.frameOffset,
                     EMPTY_CELL_VALUE);
        return emptyFrameIndex;
    }
    if(maxFrameVisited < NUM_FRAMES - 1){
        return maxFrameVisited + 1;
    }
    long long frameMaxDistance = INT64_MIN;
    // find page to evict
    return 0;
}


bool isFrameEmpty(int frameIndex) {
    for(int i = 0; i < PAGE_SIZE; i++){
        if(readByIndex(frameIndex, i) != EMPTY_CELL_VALUE) {
            return false;
        }
    }
    return true;
}

/**
 * Converts a given frame index and a given frame Offset to a physical address.
 */
uint64_t convertIndexesToAddress(uint64_t frameIndex, uint64_t frameOffset){
    return frameIndex * PAGE_SIZE + frameOffset; // TODO: verify type conversion
}


/**
 * Reads the content of a given frame index and its offset and returns the result.
 */
word_t readByIndex(int frameIndex, uint64_t frameOffset) {
    word_t result;
    PMread(convertIndexesToAddress(frameIndex, frameOffset), &result);
    return result;
}

/**
 * Write the given value in the a given frame index and its offset.
 */
void writeByIndex(int frameIndex, uint64_t frameOffset, word_t value) {
    PMwrite(convertIndexesToAddress(frameIndex, frameOffset), value);
}

int findEmptyFrame(int startFrameIndex,
                   int currDepth,
                   int *maxFrameVisited,
                   FrameAddress *parentFrame,
                   bool (*condition)(int))
                   {
    if (condition(startFrameIndex)){
        return startFrameIndex;
    }
    if(startFrameIndex > *maxFrameVisited){
        *maxFrameVisited = startFrameIndex;
    }

    if (*maxFrameVisited >= NUM_FRAMES)
        return FAILURE;

    if (currDepth >= (TABLES_DEPTH - 1)){ // TODO: -1?
        return FAILURE;
    }
    parentFrame->frameIndex = startFrameIndex; //TODO: struct containing frame index and offset
    for (int i = 0; i < PAGE_SIZE; i++){
        parentFrame->frameOffset = i;
        word_t cellValue = readByIndex(startFrameIndex, i);
        if (cellValue != EMPTY_CELL_VALUE){
            int subSearchResult = findEmptyFrame(cellValue,
                                                 currDepth + 1,
                                                 maxFrameVisited,
                                                 parentFrame,
                                                 condition);
            if (subSearchResult != -1){
                return subSearchResult;
            }
        }
    }
    return FAILURE;
                   }

                   void calcFrameCyclicDistance(int frameIndex, int swapInPageIndex, FrameAddress *frameToEvict, long long *frameMaxDistance)
                   {
    long long rowDistance = calcRowCyclicDistance(frameIndex, swapInPageIndex, i);
    if(rowDistance > *frameMaxDistance){
        // TODO update page to evict and parent?
        *frameMaxDistance = rowDistance;
    }
                   }

                   long long calcRowCyclicDistance(int frameIndex, int swapInPageIndex, int rowIndex)
                   {
    long long distance  = abs(swapInPageIndex - readByIndex(frameIndex, rowIndex));
    return std::min(NUM_PAGES - distance, distance);
                   }


                   void findFrameToEvict(int startFrameIndex,
                                         int currDepth,
                                         int swapInPageIndex,
                                         long long *currentMaxDistance,
                                         FrameAddress *parentFrame,
                                         FrameAddress *frameToEvict)
                                         {
    if (currDepth == TABLES_DEPTH){
        calcFrameCyclicDistance(startFrameIndex, swapInPageIndex, frameToEvict, currentMaxDistance); // TODO another pointer so currentMax dis would change?
        return;
    }
    parentFrame->frameIndex = startFrameIndex; //TODO: struct containing frame index and offset
    for (int i = 0; i < PAGE_SIZE; i++){
        parentFrame->frameOffset = i;
        word_t cellValue = readByIndex(startFrameIndex, i);
        if (cellValue != EMPTY_CELL_VALUE){
            findFrameToEvict(cellValue,
                             currDepth + 1,
                             swapInPageIndex,
                             currentMaxDistance,
                             parentFrame,
                             frameToEvict);
        }
    }
                                         }

