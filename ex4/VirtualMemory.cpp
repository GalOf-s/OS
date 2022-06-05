#include <algorithm>
#include "VirtualMemory.h"
#include "MemoryConstants.h"
#include "PhysicalMemory.h"


#define EMPTY_CELL_VALUE 0
#define SUCCESS 1
#define FAILURE 0
#define EMPTY_FRAME_NOT_FOUND -1

typedef struct{
    int frameIndex;
    int frameOffset;
} FrameAddress;

typedef struct{
    int frameIndex;
    int pageIndex;
    FrameAddress parentFrame;
} PageToEvict;

/**
 * Reads the content of a given frame index and its offset and returns the result.
 */
word_t readByIndex(word_t frameIndex, uint64_t frameOffset);

/**
 * Returns true if the given frame is empty, and false otherwise
 */
bool isFrameEmpty(word_t frameIndex);

/**
 * Converts a given frame index and a given frame Offset to a physical address.
 */
uint64_t convertIndexesToAddress(uint64_t frameIndex, uint64_t frameOffset);

/**
 * Return the translation of a virtual address to a physical address.
 */
uint64_t translateToPhysicalAddress(uint64_t virtualAddress);

/**
 * Calculating offsets according to according to the each level in the Table.
 */
void translateToVirtualOffsets(uint64_t pageTable[TABLES_DEPTH], uint64_t pages);

/**
 * Write the given value in the a given frame index and its offset.
 */
void writeByIndex(uint64_t frameIndex, uint64_t frameOffset, word_t value);

/**
 * Finds the next frame to use in the Table.
 */
word_t findNextFrame(word_t frame, uint64_t pageNum);

/**
 * The first translation of the virtual address according to the offsets in the page table array
 */
word_t firstTranslation(uint64_t pagesTable[TABLES_DEPTH], uint64_t pageNum);

/**
 * The first translation of the virtual address according to the last offset table array
 * (the frame in the table that holds the wanted page).
 */
word_t secondTranslation(const uint64_t pagesTable[TABLES_DEPTH], uint64_t pageNum, word_t lastFrame);

/**
 * Returns the minimum cyclic distance.
 */
long long calcCyclicDistance(word_t swapInPageIndex, word_t swapOutPageIndex);

/**
 * Finds a frame to evict.
 */
void findFrameToEvict(word_t startFrameIndex,
                      word_t currDepth,
                      word_t swapInPageIndex,
                      word_t swapOutPageIndex,
                      long long *currentMaxDistance,
                      FrameAddress *parentFrame,
                      PageToEvict *pageToEvict);

/** Finds the first frame in memory that satisfies the condition provided as a boolean function, by
 * preforming a DFS run on the page tree.
 *
 * returns the free frame's index upon success.
 * returns EMPTY_FRAME_NOT_FOUND if no empty frame was found.
 */
int findEmptyFrame(word_t startFrameIndex,
                   word_t currDepth,
                   word_t protectedFrame,
                   word_t *maxFrameVisited,
                   FrameAddress *parentFrame,
                   bool (*condition)(word_t));

/**
 * Initializes a new frame according to the given frameIndex.
 */
void initNewFrame(uint64_t frameIndex) {
    for(uint64_t i = 0; i < PAGE_SIZE; i++) {
        writeByIndex(frameIndex, i, 0);
    }
}

/**
 * Initialize the virtual memory.
 */
void VMinitialize(){
    initNewFrame(0);
//    for (int i = 0; i < NUM_FRAMES; i++){
//        initNewFrame(i);
//    }
}

/** Reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t* value) {
    if (virtualAddress >= VIRTUAL_MEMORY_SIZE)
    {
        return FAILURE;
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
    if (virtualAddress >= VIRTUAL_MEMORY_SIZE)
    {
        return FAILURE;
    }
    uint64_t physicalAddress = translateToPhysicalAddress(virtualAddress);
    PMwrite(physicalAddress, value);
    return SUCCESS;
}


void translateToVirtualOffsets(uint64_t pageTable[TABLES_DEPTH], uint64_t pages) {
    for (int i = TABLES_DEPTH - 1; i >= 0; --i) {
        pageTable[i] = pages % PAGE_SIZE;
        pages = pages >> OFFSET_WIDTH;
    }
}

uint64_t translateToPhysicalAddress(uint64_t virtualAddress) {
    uint64_t offset = virtualAddress % PAGE_SIZE;
    uint64_t pageNum = virtualAddress >> OFFSET_WIDTH;

    // an array to find the path from the root to the wanted page
    uint64_t pagesTable[TABLES_DEPTH];

    translateToVirtualOffsets(pagesTable, pageNum);

    word_t lastFrame = firstTranslation(pagesTable, pageNum);
    word_t wantedPageFrame = secondTranslation(pagesTable, pageNum, lastFrame);

    return convertIndexesToAddress(wantedPageFrame, offset);
}


word_t secondTranslation(const uint64_t pagesTable[TABLES_DEPTH], uint64_t pageNum, word_t lastFrame) {
    uint64_t lastFrameOffset = pagesTable[TABLES_DEPTH - 1];

    word_t wantedPageFrame = readByIndex(lastFrame, lastFrameOffset);
    if (wantedPageFrame == EMPTY_CELL_VALUE) {
        wantedPageFrame = findNextFrame(lastFrame, pageNum);
        PMrestore(wantedPageFrame, pageNum);
        writeByIndex(lastFrame, lastFrameOffset, wantedPageFrame);
    }
    return wantedPageFrame;
}


word_t firstTranslation(uint64_t pagesTable[TABLES_DEPTH], uint64_t pageNum) {
    word_t currentFrame = 0;

    int i = 0;
    while (i < TABLES_DEPTH - 1) {
        word_t nextFrame = readByIndex(currentFrame, pagesTable[i]);
        if(nextFrame == EMPTY_CELL_VALUE) {
            nextFrame = findNextFrame(currentFrame, pageNum);
            initNewFrame(nextFrame);
            writeByIndex(currentFrame, pagesTable[i], nextFrame);
        }
        currentFrame = nextFrame;
        i++;
    }
    return currentFrame;
}


word_t findNextFrame(word_t protectedFrame, uint64_t pageNum) {
    word_t maxFrameVisited = 0;
    FrameAddress parentFrame;
    int emptyFrameIndex = findEmptyFrame(0,
                                         0,
                                         protectedFrame,
                                         &maxFrameVisited,
                                         &parentFrame,
                                         isFrameEmpty);
    if (emptyFrameIndex != EMPTY_FRAME_NOT_FOUND){
        writeByIndex(parentFrame.frameIndex, parentFrame.frameOffset,
                     EMPTY_CELL_VALUE);
        return emptyFrameIndex;
    }
    if(maxFrameVisited < NUM_FRAMES - 1){
        return maxFrameVisited + 1;
    }
    long long frameMaxDistance = INT64_MIN;
    PageToEvict pageToEvict;
    findFrameToEvict(0,
                     0,
                     (word_t) pageNum,
                     0,
                     &frameMaxDistance,
                     &parentFrame,
                     &pageToEvict);
    PMevict(pageToEvict.frameIndex, pageToEvict.pageIndex);
    writeByIndex(pageToEvict.parentFrame.frameIndex,
                 pageToEvict.parentFrame.frameOffset,
                 EMPTY_CELL_VALUE);
    return pageToEvict.frameIndex;
}


bool isFrameEmpty(word_t frameIndex) {
    for(word_t i = 0; i < PAGE_SIZE; i++){
        if(readByIndex(frameIndex, i) != EMPTY_CELL_VALUE) {
            return false;
        }
    }
    return true;
}

uint64_t convertIndexesToAddress(uint64_t frameIndex, uint64_t frameOffset){
    return frameIndex * PAGE_SIZE + frameOffset;
}


word_t readByIndex(int frameIndex, uint64_t frameOffset) {
    word_t result;
    PMread(convertIndexesToAddress(frameIndex, frameOffset), &result);
    return result;
}


void writeByIndex(uint64_t frameIndex, uint64_t frameOffset, word_t value) {
    PMwrite(convertIndexesToAddress(frameIndex, frameOffset), value);
}

word_t findEmptyFrame(word_t startFrameIndex,
                      word_t currDepth,
                      word_t protectedFrame,
                      word_t *maxFrameVisited,
                      FrameAddress *parentFrame,
                      bool (*condition)(word_t))
{
    if(startFrameIndex > *maxFrameVisited){
        *maxFrameVisited = startFrameIndex;
    }

    if (currDepth == TABLES_DEPTH || *maxFrameVisited >= NUM_FRAMES) {
        return EMPTY_FRAME_NOT_FOUND;
    }

    if (condition(startFrameIndex) && protectedFrame != startFrameIndex){
        return startFrameIndex;
    }

    for (int i = 0; i < PAGE_SIZE; i++) {
        word_t cellValue = readByIndex(startFrameIndex, i);
        parentFrame->frameIndex = startFrameIndex;
        parentFrame->frameOffset = i;
        if (cellValue != EMPTY_CELL_VALUE) {
            int subSearchResult = findEmptyFrame(cellValue,
                                                 currDepth + 1,
                                                 protectedFrame,
                                                 maxFrameVisited,
                                                 parentFrame,
                                                 condition);

            if (subSearchResult != EMPTY_FRAME_NOT_FOUND) {

                return subSearchResult;
            }
        }
    }
    return EMPTY_FRAME_NOT_FOUND;
}


void findFrameToEvict(word_t startFrameIndex,
                      word_t currDepth,
                      word_t swapInPageIndex,
                      word_t swapOutPageIndex,
                      long long *currentMaxDistance,
                      FrameAddress *parentFrame,
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
                             (swapOutPageIndex << OFFSET_WIDTH) + i,
                             currentMaxDistance,
                             parentFrame,
                             pageToEvict);
        }
    }
}

long long calcCyclicDistance(word_t swapInPageIndex, word_t swapOutPageIndex)
{
    long long distance  = abs(swapInPageIndex - swapOutPageIndex);
    return std::min(NUM_PAGES - distance, distance);
}


