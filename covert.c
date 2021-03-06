#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <string.h>

#include "covert.h"

#define WRITE_FILE      "./file.txt"
#define TIMER           ITIMER_REAL
#define TIMER_SIGNAL    SIGALRM

#define WRITE_LEN_SEND  1048576
#define WRITE_LEN_REC  104857

volatile sig_atomic_t flag;
//Assumes WRITE_LEN_SEND > WRITE_LEN_REC
static char buf[WRITE_LEN_SEND];

/**
    Timer interrupt service routine. Raises flag indicating timer reached.
*/
void bit_timer_isr(int sig) { flag = 1; }


/**
    Returns that amount of time (useconds) to write to a file.
*/
long covert_read_time()
{
    int             file, num;
    struct timeval  start, end;
    long            elapsed;
    clock_t         clock_start;

    /* Get start time */
    if( gettimeofday( &start, NULL ) < 0 )
    {
        perror( "covert_read_time: gettimeofday error" );
        exit(1);
    }

/*    clock_start = clock();
*/
    /* Open file for writing, clear any existing content, create if necessary */
    if ( (file = open( WRITE_FILE, O_WRONLY|O_TRUNC|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP )) < 0 )
    {
        perror( "covert_read_time: fopen error" );
        exit(1);
    }
    /* Write to file */
    num = 0;
    while ( num < WRITE_LEN_REC )
    {
        if ( (num += write( file, buf, WRITE_LEN_REC-num )) < 0 )
        {
            perror( "covert_read_time: fwrite error" );
            exit(1);
        }
    }
    /* Close file */
    if( close( file ) < 0 )
    {
        perror( "covert_read_time: fclose error" );
        exit(1);
    }

    /* Get end time */
    if( gettimeofday( &end, NULL ) < 0 )
    {
        perror( "covert_read_time: gettimeofday error" );
        exit(1);
    }

    /* Calculate elapsed time */
    //TODO (At what point would this overflow?)
    elapsed = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);

/*    clock_start = clock() - clock_start;
    return clock_start;
*/    return elapsed;
}


/** 
    Write to disk for given period of time (useconds).
    value: Bit value
    period: Amount of time to write (useconds).
*/
void covert_write_bit( int value, long period )
{
    int                 file;
    int                 result, num;

    struct itimerval    timer, old_timer, empty_timer;
    struct sigaction    sa, old_sa;

    /* For 0 bit, wait. */
    if ( !value )
    {
        sleep( period / 1000000 );
        usleep( period % 1000000 );
        return;
    }

    /* Clear flag */
    flag = 0;

    /* Configure interrupt service routine */
    sa.sa_handler = bit_timer_isr;
    sa.sa_flags = 0;
    sigemptyset( &sa.sa_mask );
    if ( sigaction(TIMER_SIGNAL, &sa, &old_sa) < 0 )
    {
        perror( "covert_write_bit: sigaction error; setting sigaction" );
        exit(1);
    }

    /* Set timer to threshold value */
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    timer.it_value.tv_sec = period / 1000000;
    timer.it_value.tv_usec = period % 1000000;
    if ( setitimer( TIMER, &timer, &old_timer ) < 0 ) 
    {
        perror( "covert_write_bit: setitimer error; setting timer" );
        exit(1);
    }

    /* Begin writing to file. If any of these methods are interrupted by the 
       timer, bit_timer_isr will be called and the flag will be set. */
    while(1)
    {
        if ( flag ) goto FINISH_WRITE;

        /* Open file for writing, clear any existing content, create if necessary */
        if ( (file = open( WRITE_FILE, O_WRONLY|O_TRUNC|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP )) < 0 )
        {
            if ( errno == EINTR ) goto FINISH_WRITE;
            perror( "covert_write_bit: fopen error" );
            exit(1);
        }
        /* Write to file */
        num = 0;
        while ( num < WRITE_LEN_SEND )
        {
            if ( flag ) goto FINISH_WRITE;
            if ( (num += write( file, buf, WRITE_LEN_SEND-num )) < 0 )
            {
                if ( errno == EINTR ) goto FINISH_WRITE;
                perror( "covert_write_bit: fwrite error" );
                exit(1);
            }
        }
        /* Flush write */
        if ( flag ) goto FINISH_WRITE;
        if ( close( file ) < 0 )
        {
            if ( errno == EINTR ) goto FINISH_WRITE;
            perror( "covert_write_bit: close error" );
            exit(1);
        }
        file = 0;

    }

FINISH_WRITE:
    /* Close file */
    if( file && close( file ) < 0 )
    {
        if ( errno == EBADF ) goto FINISH;
        perror( "covert_write_bit: error closing file" );
        exit(1);
    }
FINISH:
    /* Replace old timer and sigaction */
    if ( sigaction(TIMER_SIGNAL, &old_sa, NULL) < 0 )
    {
        perror( "covert_write_bit: sigaction error; restoring sigaction" );
        exit(1);
    }
    if ( setitimer( TIMER, &old_timer, NULL ) < 0 ) 
    {
        perror( "covert_write_bit: setitimer error; " );
        exit(1);
    }

}
