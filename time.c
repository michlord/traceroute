#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
/*
* This function returns current time in seconds stored in double and
* with most precision possible.
*/
double get_time(){
    struct timeval tv;
    double time;
    
    gettimeofday(&tv,0);
    time = ((double) tv.tv_usec) / 1000000. + (unsigned long) tv.tv_sec;
    
    return time;
}