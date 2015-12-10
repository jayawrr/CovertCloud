#include <stdlib.h>
#include <stdio.h>

#include "covert.h"

#define THRESHOLD   10000
#define SAMPLES     1
#define PERIOD 	    2000000
#if PERIOD < (THRESHOLD*SAMPLES)
// I forget what's the actual way to assert something, this
//works for now
exit
#endif

int main( int argc, char *argv[] )
{
    int i, mode, time[SAMPLES];
    struct timeval  start, end;
    long            elapsed;
    clock_t         clock_start;

    /* Get start time */
    if( gettimeofday( &start, NULL ) < 0 )
    {
        perror( "covert_read_time: gettimeofday error" );
        exit(1);
    }
    while(1) {
        int bit = 1;
        /* Get start time */
        if( gettimeofday( &start, NULL ) < 0 ) {
            perror( "covert_read_time: gettimeofday error" );
            exit(1);
	}
	while(1) {
            long ret = covert_read_time();
            if (ret > THRESHOLD) { bit = 0;}
	    
            /* Get end time */
            if( gettimeofday( &end, NULL ) < 0 )
            {
                perror( "covert_read_time: gettimeofday error" );
                exit(1);
            }
            usleep(10000); // TODO: hacked in from covert_test, is this neccessary?
            /* Calculate elapsed time */
            elapsed = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
            if(elapsed > PERIOD) { 
                break;
            }
        }
	printf("%d\n",bit); 
    }
return 0;

#if 0
    while(1)
    {
        mode = 0;
        for( i = 0; i < SAMPLES; i++ )
        {
            time[i] = covert_read_bit( THRESHOLD );
            mode += time[i];
            /*printf( "%d\n", time );
            fflush(NULL);
            usleep( 10000 );*/
        }
        mode /= SAMPLES/2+1;
        printf( "%d\n", mode );
        fflush(NULL);
        usleep(PERIOD-SAMPLES*THRESHOLD);
    }

    return 0;
#endif
}
