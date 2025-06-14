#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include <sys/time.h>

typedef struct {
  struct timespec start;
  struct timespec stop;
} CTimer;

// Starts the timer
void start_timer(CTimer *timer);

// Stops the timer
void stop_timer(CTimer *timer);

// Gets the elapsed time in microseconds
double get_elapsed_time_microseconds(CTimer *timer);


#endif // TIMER_H