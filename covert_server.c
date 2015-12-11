
#include <stdlib.h>
#include <stdio.h>

#include "covert.h"

int main( int argc, char *argv[] )
{
    int i, value;

    value = 0;

    for( i = 0; i < 100; i++ )
    {
        covert_write_bit( value ^= 1, 2050000 );
        printf( "%d\n", value );
   }

    return 0;
}
