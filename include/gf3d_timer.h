#ifndef _GF3D_TIMER_H_
#define _GF3D_TIMER_H_

#include <time.h>

typedef struct Timer_s
{
    clock_t startTicks;
    clock_t pausedTicks;

    int paused;
    int started;
} Timer;

Timer gf3d_timer_new();

void gf3d_timer_start(Timer *timer);

void gf3d_timer_stop(Timer *timer);

void gf3d_timer_pause(Timer *timer);

void gf3d_timer_unpause(Timer *timer);

float gf3d_timer_get_ticks(Timer *timer);

#endif