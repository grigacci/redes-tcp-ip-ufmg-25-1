#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include "timer.h"

// POSIX implementation
void start_timer(CTimer *timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->start);
}

void stop_timer(CTimer *timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->stop);
}

double get_elapsed_time_microseconds(CTimer *timer) {
    double start_us = (double)timer->start.tv_sec * 1000000.0 + (double)timer->start.tv_nsec / 1000.0;;
    double stop_us = (double)timer->stop.tv_sec * 1000000.0 + (double)timer->stop.tv_nsec / 1000.0;
    return stop_us - start_us;
}