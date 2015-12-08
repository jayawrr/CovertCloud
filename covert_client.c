#include <stdlib.h>
#include <stdio.h>

#include "covert.h"

#define THRESHOLD   500
#define SAMPLES     5

int main( int argc, char *argv[] )
{
    int i, mode, time[SAMPLES];

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
        usleep(1000000-SAMPLES*THRESHOLD);
    }

    return 0;
}