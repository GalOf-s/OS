#include "osm.h"
#include <iostream>
#include <sys/time.h>

#define UNROLLING_FACTOR 10
#define MILLION 1000000
double osm_syscall_time(unsigned int iterations){
	if(iterations <= 0) return -1;

	struct timeval start_time;
	gettimeofday(&start_time, nullptr);

	int i;
	for(i=0; i<iterations; i+=UNROLLING_FACTOR){
		OSM_NULLSYSCALL;
		OSM_NULLSYSCALL;
		OSM_NULLSYSCALL;
		OSM_NULLSYSCALL;
		OSM_NULLSYSCALL;
		OSM_NULLSYSCALL;
		OSM_NULLSYSCALL;
		OSM_NULLSYSCALL;
		OSM_NULLSYSCALL;
		OSM_NULLSYSCALL;
	}

	struct timeval end_time;
	gettimeofday(&end_time, nullptr);

	double total_time = end_time.tv_sec - start_time.tv_sec + (end_time.tv_usec - start_time
			.tv_usec)/MILLION
	return total_time / i;
}



