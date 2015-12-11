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

// Find out how many periods occur in the duration t:
time_t series_length(struct timeval *t) {
// Ideally we would do division between timeval, but let's not bother for this.
// Ugh, instead for now let's do the following:
// With a period of 2 seconds, a samp_period of 2.3 seconds, 0bit periods 
// very rarely go over 3 seconds, and 1 bit periods rarely go under 2 seconds.
// Let's just round anything over 1 second up to two seconds, and round down
// anything under 1 seconds.
  time_t mod = t->tv_sec % PERIOD_SEC;
  time_t ret = (t->tv_sec - mod) / PERIOD_SEC;
  if (mod > 1) {
     ret += 1;
  }
  return ret;
}

// Moves any time that is in excess of a multiplier of the PERIOD from source
// to dest.
// This is usefull for measurements which bias towards source.
// For e.g, the data often shows: period = 2, dest = 1.6, source = 2.4
// this will set dest = (2.4 - 2) + 1.6 =2 and set source = 2
// NOTE: this works well for square wave, untested for other signals.
void balance_timing(struct timeval *dest, struct timeval *source) {
  struct timeval period;
  struct timeval excess;
  struct timeval temp;
  struct timeval zero;
  zero.tv_sec = 0;
  zero.tv_usec = 0;
  period.tv_sec = PERIOD_SEC;
  period.tv_usec = PERIOD_USEC;

  temp.tv_usec = source->tv_usec;
  temp.tv_sec  = source->tv_sec % (2 * PERIOD_SEC);
  timersub(&temp, &period, &excess); 
  

  // Is there any excess?:
  if (timercmp(&excess, &zero, >)) {
    timersub(source, &excess, source);
    timeradd(&excess, dest, dest);
  }
}

int main( int argc, char *argv[] )
{
  struct timeval  l_start, h_start, h_end, curr, samp_period, l_diff, h_diff;
  struct timeval prevt;
  long            elapsed;
  int count;
  int prev_bit = 0;
  samp_period.tv_sec = 0;
  samp_period.tv_usec = SAMP_PERIOD;

  // Initialize our start times:
  gettime(&l_start);
  h_start = l_start;
  h_end = l_start;

  int bit = 0;
  int prevbit = 0;
  time_t length;
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
          // Ther's a small bias towards l_diff
          // balance_timing tries to balance things out a little.
          balance_timing(&h_diff, &l_diff);

          length = series_length(&h_diff);
          for (count = 0; count < length; count++) {
            printf("1\n");
          }
        }
        // Reset high_start time
        //gettime(&h_start);
        // set it to prevt to not ignore the time of the current read. 
        h_start = prevt;
        // print the series of 0 bits.
        //printf("l_diff: %ld\n", l_diff.tv_sec);
        //printf("l_diff: %ld\n", l_diff.tv_usec);
          length = series_length(&l_diff);
          for (count = 0; count < length; count++) {
            printf("0\n");
          }
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
