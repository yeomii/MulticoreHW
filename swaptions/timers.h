#ifndef __TIMERS__
#define __TIMERS__

void init_timers();
void start_timer(int i);
void stop_timer(int i);
void clear_timer(int i);
float read_timer(int i);
int read_count(int i);

#endif
