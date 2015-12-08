
#ifndef COVERT_H
#define COVERT_H

/**
    Timer interrupt service routine. Raises flag indicating timer reached.
*/
void bit_timer_isr( int sig );

/**
    Attempts to write to disk within given threshold of time (useconds).
    Returns 1 if time taken is longer than threshold, 0 otherwise. 
*/
int covert_read_bit( long threshold );

/**
    Returns that amount of time (useconds) to write to a file.
*/
long covert_read_time();

/**
    Write to disk for given period of time.
    value: Bit value
    period: Amount of time to write (useconds).
*/
void covert_write_bit( int value, long period );

#endif
