#include <algorithm>
#include <cassert>
#include "VirtualMemory.h"
#include "MemoryConstants.h"
#include "PhysicalMemory.h"


#define EMPTY_CELL_VALUE 0
#define SUCCESS 1
#define FAILURE 0

typedef struct{
    int frameIndex;
    int frameOffset;
} FrameAddress;

typedef struct{
    int frameIndex;
    int pageIndex;
    FrameAddress parentFrame;
} PageToEvict;

int readByIndex(int frameIndex, uint64_t frameOffset);


bool isFrameEmpty(int frameIndex);

uint64_t convertIndexesToAddress(uint64_t frameIndex, uint64_t frameOffset);

uint64_t translateToPhysicalAddress(uint64_t virtualAddress);

void translateToVirtualOffsets(uint64_t pageTable[], uint64_t pages);

void writeByIndex(int frameIndex, uint64_t frameOffset, word_t value);


word_t findNextFrame(word_t frame, uint64_t pageNum, int searchStartDepth);

word_t firstTranslation(uint64_t pagesTable[], uint64_t pageNum);

word_t secondTranslation(const uint64_t pageTable[], uint64_t pageNum, word_t lastFrame);

long long calcCyclicDistance(int swapInPageIndex, int swapOutPageIndex);

void findFrameToEvict(int startFrameIndex,
                      int currDepth,
                      int swapInPageIndex,
                      int swapOutPageIndex,
                      long long *currentMaxDistance,
                      FrameAddress *parentFrame,
                      PageToEvict *pageToEvict);
/** Finds the first frame in memory that satisfies the condition provided as a boolean function, by
 * preforming a DFS run on the page tree.
 *
 * returns the free frame's index upon success.
 * returns -1 if no empty frame was found.
 */
int findEmptyFrame(int startFrameIndex,
                   int currDepth,
                   word_t protectedFrame,
                   word_t *maxFrameVisited,
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
    if (TABLES_DEPTH == 0)
    {
        PMread(virtualAddress, value);
        return SUCCESS;
    }
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
    if (TABLES_DEPTH == 0)
    {
        PMwrite(virtualAddress, value);
        return SUCCESS;
    }
    uint64_t physicalAddress = translateToPhysicalAddress(virtualAddress);
    PMwrite(physicalAddress, value);
    return SUCCESS;
}


void translateToVirtualOffsets(uint64_t pageTable[], uint64_t pages) {
    for (int i = TABLES_DEPTH; i >= 0; --i) {
        pageTable[i - 1] = pages % PAGE_SIZE; // TODO check if this is the right macro to get the pages: [p1, p2]
        pages = pages >> OFFSET_WIDTH;
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

    word_t lastFrame = firstTranslation(pagesTable, pageNum);
    word_t wantedPageFrame = secondTranslation(pagesTable, pageNum, lastFrame);

    return convertIndexesToAddress(wantedPageFrame, offset);
}

word_t secondTranslation(const uint64_t pagesTable[], uint64_t pageNum, word_t lastFrame) {
    uint64_t lastFrameOffset = pagesTable[TABLES_DEPTH - 1];

    word_t wantedPageFrame = readByIndex(lastFrame, lastFrameOffset); // addr2
    if (wantedPageFrame == EMPTY_CELL_VALUE) {
        wantedPageFrame = findNextFrame(lastFrame, pageNum,0);
        PMrestore(wantedPageFrame, pageNum);
        writeByIndex(lastFrame, lastFrameOffset, wantedPageFrame);
    }
    return wantedPageFrame;
}

word_t firstTranslation(uint64_t pagesTable[], uint64_t pageNum) {
    word_t currentFrame = 0; //addr1

    int i = 0;
    while (i < TABLES_DEPTH - 1) {
        word_t nextFrame = readByIndex(currentFrame, pagesTable[i]);
        if(nextFrame == EMPTY_CELL_VALUE) {
            nextFrame = findNextFrame(currentFrame, pageNum, i);
            initNewFrame(nextFrame);
            writeByIndex(currentFrame, pagesTable[i], nextFrame);
        }
        currentFrame = nextFrame;
        i++;
    }
    return currentFrame;
}


word_t findNextFrame(word_t protectedFrame, uint64_t pageNum, int searchStartDepth) {
    //TODO: needs to know which frames are needed for the current pageNum
    word_t maxFrameVisited = 0; //TODO: frame
    FrameAddress parentFrame;
//    FrameAddress parentResult;
    int emptyFrameIndex = findEmptyFrame(0, 0,
										 protectedFrame,
                                         &maxFrameVisited,
                                         &parentFrame,
                                         isFrameEmpty); // TODO: should always start from 0
    if (emptyFrameIndex != -1){
        writeByIndex(parentFrame.frameIndex, parentFrame.frameOffset,
                     EMPTY_CELL_VALUE);
        return emptyFrameIndex;
    }
    if(maxFrameVisited < NUM_FRAMES - 1){
        return maxFrameVisited + 1;
    }
    long long frameMaxDistance = INT64_MIN;
    PageToEvict pageToEvict;
    findFrameToEvict(0, 0, (int) pageNum, 0, &frameMaxDistance, &parentFrame, &pageToEvict);
    assert(pageToEvict.pageIndex < NUM_PAGES);
    PMevict(pageToEvict.frameIndex, pageToEvict.pageIndex);
    writeByIndex(pageToEvict.parentFrame.frameIndex, pageToEvict.parentFrame.frameOffset,
                 EMPTY_CELL_VALUE);
    return pageToEvict.frameIndex;
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
                   word_t protectedFrame,
                   word_t *maxFrameVisited,
                   FrameAddress *parentFrame,
                   bool (*condition)(int))
{
	if(startFrameIndex > *maxFrameVisited){
		*maxFrameVisited = startFrameIndex;
	}

	if (currDepth == TABLES_DEPTH){ // TODO: -1?
		return -1;
	}

	if (condition(startFrameIndex) && protectedFrame != startFrameIndex){
		return startFrameIndex;
	}


    if (*maxFrameVisited >= NUM_FRAMES)
        return -1;


    for (int i = 0; i < PAGE_SIZE; i++){
        word_t cellValue = readByIndex(startFrameIndex, i);
        parentFrame->frameIndex = startFrameIndex; //TODO: struct containing frame index and offset
        parentFrame->frameOffset = i;
        if (cellValue != EMPTY_CELL_VALUE){
            int subSearchResult = findEmptyFrame(cellValue,
                                                 currDepth + 1,
                                                 protectedFrame,
                                                 maxFrameVisited,
                                                 parentFrame,
                                                 condition);

            if (subSearchResult != -1){

                return subSearchResult;
            }
        }
    }


    return -1;
}

void findFrameToEvict(int startFrameIndex, int currDepth, int swapInPageIndex, int swapOutPageIndex,
                      long long int *currentMaxDistance, FrameAddress *parentFrame,
                      PageToEvict *pageToEvict)
{
    if (currDepth == TABLES_DEPTH){
        long long distance = calcCyclicDistance(swapInPageIndex, swapOutPageIndex);
        if(distance > *currentMaxDistance){
            *currentMaxDistance = distance;
            pageToEvict->frameIndex = startFrameIndex;
            pageToEvict->pageIndex = swapOutPageIndex;
            pageToEvict->parentFrame = *parentFrame;
        }
        return;
    }
    parentFrame->frameIndex = startFrameIndex;
    for (int i = 0; i < PAGE_SIZE; i++){
        parentFrame->frameOffset = i;
        word_t cellValue = readByIndex(startFrameIndex, i);
        if (cellValue != EMPTY_CELL_VALUE){
            findFrameToEvict(cellValue,
                             currDepth + 1,
                             swapInPageIndex,
                             (swapOutPageIndex<<(int) log2(PAGE_SIZE)) + i,
                             currentMaxDistance,
                             parentFrame,
                             pageToEvict);
        }
    }
}

long long calcCyclicDistance(int swapInPageIndex, int swapOutPageIndex)
{
    long long distance  = abs(swapInPageIndex - swapOutPageIndex);
    return std::min(NUM_PAGES - distance, distance);
}





