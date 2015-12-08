
#include <stdlib.h>
#include <stdio.h>

#include "covert.h"

int main( int argc, char *argv[] )
{
    long time;

    while(1)
    {
        time = covert_read_time();
        printf( "%ld\n", time );
        fflush(NULL);
        usleep( 10000 );
    }

    return 0;
}