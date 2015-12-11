
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "covert.h"

int main( int argc, char *argv[] )
{
    int i, value;
    srand(time(NULL));

    value = 0;

    for( i = 0; i < 1000; i++ )
    {
	value = rand() % 2;
        covert_write_bit( value, 2000000 );
        printf( "%d\n", value );
   }

    return 0;
}
