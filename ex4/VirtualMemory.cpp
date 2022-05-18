#include "VirtualMemory.h"
#include "MemoryConstants.h"
#include "PhysicalMemory.h"


# define EMPTY_CELL_VALUE 0
/*
 * Initialize the virtual memory.
 */
void VMinitialize(){

}

/* Reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t* value){
	return 0;
}

/* Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite(uint64_t virtualAddress, word_t value){
	return 0;
}


int ReadByIndex(int frameIndex, int frameOffset);


bool isFrameEmpty(int frameIndex){
	for(int i=0; i<PAGE_SIZE; i++){
		if(ReadByIndex(frameIndex, i) != EMPTY_CELL_VALUE){
			return false;
		}
	}
	return true;
}

uint64_t convertIndexesToAddress(int frameIndex, int frameOffset){
	return (uint64_t) frameIndex*PAGE_SIZE + frameOffset; // TODO: verify type conversion
}

/* Finds the first frame in memory that satisfies the condition provided as a boolean function, by
 * preforming a DFS run on the page tree.
 *
 * returns the free frame's index upon success.
 * returns -1 if no empty frame was found.
 */
int findSatisfyingFrame(int startFrameIndex, int currDepth, int* maxFrameVisited, bool (*condition)
(int)){
	if (condition(startFrameIndex)){
		return *maxFrameVisited + 1;
	}
	if(startFrameIndex > *maxFrameVisited){
		*maxFrameVisited = startFrameIndex;
	}
	if (*maxFrameVisited >= NUM_FRAMES)
		return -1;

	if (currDepth < (TABLES_DEPTH-1)){
		return -1;
	}
	for (int i=0; i<PAGE_SIZE; i++){
		word_t cell_value = ReadByIndex(startFrameIndex, i);
		if (cell_value != EMPTY_CELL_VALUE){
			int sub_search_result = findSatisfyingFrame(cell_value, currDepth + 1, maxFrameVisited,
											   condition);
			if (sub_search_result != -1){
				return sub_search_result;
			}
		}
	}
	return -1;
}

int ReadByIndex(int frameIndex, int frameOffset){
	word_t result;
	PMread(convertIndexesToAddress(frameIndex, frameOffset), &result);
	return result;
}

