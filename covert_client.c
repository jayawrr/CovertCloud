#include <stdlib.h>
#include <stdio.h>
#include "sys/time.h"

#include "covert.h"

#define THRESHOLD   7500
#define PERIOD_SEC  2
#define PERIOD_USEC 0
// length of time of consecutive low reads which mark the end of 1 bits.
// data has show to be biased to 0s, therefore try to counteract
// the bias.
#define SAMP_PERIOD 2300000

void gettime(struct timeval *t) {
  if( gettimeofday( t, NULL ) < 0 )
  {
      perror( "gettimeofday error" );
      exit(1);
  }
}

int series_length(struct timeval *t) {
 
}

// Moves any time that is in excess of a multiplier of the PERIOD from source
// to dest.
// This is usefull for measurements which bias towards source.
// For e.g, the data often shows: period = 2, dest = 1.6, source = 2.4
// this will set dest = (2.4 - 2) + 1.6 =2 and set source = 2
void balance_timing(struct timeval *dest, struct timeval *source) {
  struct timeval period;
  struct timeval excess;
  period.tv_sec = PERIOD_SEC;
  period.tv_usec = PERIOD_USEC;
  timersub(source, &period, &excess); 
  //TODO don't assume excess is positive. Data shows almost always postive so ok.
  timeradd(&excess, dest, dest);
}

int main( int argc, char *argv[] )
{
  struct timeval  l_start, h_start, h_end, curr, samp_period, l_diff, h_diff;
  struct timeval prevt;
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
      timersub(&curr, &l_start, &l_diff); 
      if(timercmp(&l_diff, &samp_period, >)) {
        // The rising edge end signals the end of a series of 0 bits.
        
        if(timercmp(&h_end, &h_start, !=)) {
          // We also use this opportunity to print out the series of 1 bits
          // that preceded this series of 0 bits.
          timersub(&h_end, &h_start, &h_diff);
          balance_timing(&h_diff, &h_diff);

          
          printf("h_diff: %ld\n", h_diff.tv_sec);
          printf("h_diff: %ld\n", h_diff.tv_usec);
        }
        // Reset high_start time
        //gettime(&h_start);
        // set it to prevt to not ignore the time of the current read. 
        h_start = prevt;
        // print the series of 0 bits.
        printf("l_diff: %ld\n", l_diff.tv_sec);
        printf("l_diff: %ld\n", l_diff.tv_usec);
      } 
    } else if(!bit && prevbit == 1) {
      // Case: falling edge
      h_end = curr;
      l_start = curr;
    }
    prevbit = bit;
    gettime(&prevt);
    usleep(10000); // To prevent overburdenning the channel from this end.
  } //end while
  return 0;
}
