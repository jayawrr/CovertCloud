#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>

#include "covert.h"

#define WRITE_FILE      "./file.txt"
#define WRITE_STRING    "This string is 25 bytes\n"
#define WRITE_LEN       strlen(WRITE_STRING)+1
#define TIMER           ITIMER_REAL
#define TIMER_SIGNAL    SIGALRM

volatile sig_atomic_t flag;
volatile sig_atomic_t threshold;
/**
    Timer interrupt service routine. Raises flag indicating timer reached.
*/
void bit_timer_isr( int sig ) { flag = 1; }

/**
    Initializes program for covert communication.
*/
void covert_start( int threshold, int period )
{
    struct sigaction    sa;
    struct itimerval    timer;

    /* Configure interrupt service routine */
    sa.sa_handler = covert_phy_bit;
    sa.sa_flags = 0;
    sigemptyset( &sa.sa_mask );
    if ( sigaction(SIGPROF, &sa, NULL) < 0 )
    {
        perror( "covert_start: sigaction error" );
        exit(1);
    }

    /* Set timer */
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = period;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = period;
    if ( setitimer( TIMER, &timer, NULL ) < 0 )
    {
        perror( "covert_start: setitimer error" );
        exit(1);
    }
}

/** 
    Attempts to write to disk within given threshold of time (useconds).
    Returns 1 if time taken is longer than threshold, 0 otherwise.
*/
int covert_phy_bit()
{
    int                 file;
    int                 result, num;

    struct itimerval    timer, old_timer, empty_timer;
    struct sigaction    sa, old_sa;

    /* Clear flag */
    flag = 0;

    /* Configure interrupt service routine */
    sa.sa_handler = bit_timer_isr;
    sa.sa_flags = 0;
    sigemptyset( &sa.sa_mask );
    if ( sigaction(SIGPROF, &sa, &old_sa) < 0 )
    {
        perror( "covert_phy_bit: sigaction error" );
        exit(1);
    }

    /* Set timer to threshold value */
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = threshold;
    if ( setitimer( TIMER, &timer, &old_timer ) < 0 ) 
    {
        perror( "covert_phy_bit: setitimer error; bit timer set" );
        exit(1);
    }

    /* Begin writing to file. If any of these methods are interrupted by the 
       timer, bit_timer_isr will be called and the flag will be set. */
    /* Open file for writing, clear any existing content, create if necessary */
    if ( flag ) goto FINISH_READ;
    if ( file = open( WRITE_FILE, O_WRONLY|O_TRUNC|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP ) == NULL )
    {
        if ( errno == EINTR ) goto FINISH_READ;
        perror( "covert_phy_bit: fopen error" );
        exit(1);
    }
    /* Write to file */
    num = 0;
    while ( num < WRITE_LEN )
    {
        if ( flag ) goto FINISH_READ;
        if ( num += write( file, WRITE_STRING, WRITE_LEN-num ) < 0 )
        {
            if ( errno == EINTR ) goto FINISH_READ;
            perror( "covert_phy_bit: fwrite error" );
            exit(1);
        }
    }
    /* Close file */
    if ( flag ) goto FINISH_READ;
    if ( close( file ) < 0 )
    {
        if ( errno == EINTR ) goto FINISH_READ;
        perror( "covert_phy_bit: fclose error" )
        exit(1);
    }

    /* Cancel timer */
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    if ( setitimer( TIMER, &timer, NULL ) < 0 ) 
    {
        perror( "covert_phy_bit: setitimer error; bit timer set" );
        exit(1);
    }


FINISH_READ:
    /* Replace old timer and sigaction */
    if ( sigaction(SIGPROF, &old_sa, NULL) < 0 )
    {
        perror( "covert_phy_bit: sigaction error" );
        exit(1);
    }
    if ( setitimer( TIMER, &old_timer, NULL ) < 0 ) 
    {
        perror( "covert_phy_bit: setitimer error; " )
        exit(1);
    }

    /* Return status of flag */
    result = flag;
    return result;
}


