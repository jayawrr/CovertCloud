#include <stdlib.h>
#include <stdio.h>
#include "sys/time.h"

#include "covert.h"

#define THRESHOLD   10000
#define PERIOD 	    2000000
#define SAMP_PERIOD 1500000

void gettime(struct timeval *t) {
  if( gettimeofday( t, NULL ) < 0 )
  {
      perror( "gettimeofday error" );
      exit(1);
  }
}

int main( int argc, char *argv[] )
{
  struct timeval  l_start, h_start, h_end, curr, samp_period, diff;
  long            elapsed;
  int prev_bit = 0;
  samp_period.tv_sec = 0;
  samp_period.tv_usec = SAMP_PERIOD;

  // Initialize our start times:
  gettime(&l_start);
  h_start = l_start;
  h_end = l_start;

  int bit = 0;
  int prevbit = 0;
  // Signal parsing loop:
  while(1) {
    // Time a read:
    long ret = covert_read_time();
    if (ret > THRESHOLD) {
      bit = 1;
    } else {
      bit = 0;
    }
    gettime(&curr);
    if(bit && prevbit == 0) {
      // Case: rising edge
      timersub(&curr, &l_start, &diff); 
      if(timercmp(&diff, &samp_period, >)) {
        // The rising edge end signals the end of a series of 0 bits.
        
        if(timercmp(&h_end, &h_start, !=)) {
          // We also use this opportunity to print out the series of 1 bits
          // that preceded this series of 0 bits.
          timersub(&h_end, &h_end, &diff);
          printf("h_diff: %ld\n", diff.tv_usec);
        }
        // Reset high_start time
        gettime(&h_start);
        // print the series of 0 bits.
        timersub(&curr, &l_start, &diff); 
        printf("l_diff: %ld\n", diff.tv_usec);
      } 
    } else if(!bit && prevbit == 1) {
      // Case: falling edge
      gettime(&h_end);
      l_start = h_end;
    }
    usleep(10000); // To prevent overburdenning the channel from this end.
    prevbit = bit;
  } //end while
  return 0;
}
