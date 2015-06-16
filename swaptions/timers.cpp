#include <chrono>
#include "timers.h"

#define TIMERS 8

using namespace std;
using namespace chrono;

static system_clock::time_point timers[TIMERS];
static float elapsed[TIMERS];
static int count[TIMERS];

void init_timers()
{
  for (int i=0; i<TIMERS; i++)
  {
    timers[i] = system_clock::now();
    elapsed[i] = 0;
    count[i] = 0;
  }
}

void start_timer(int i)
{
  timers[i] = system_clock::now();
  count[i]++;
}

void stop_timer(int i)
{
  system_clock::time_point end = system_clock::now();
  elapsed[i] += duration_cast<duration<float, milli>>(end - timers[i]).count();
}

void clear_timer(int i)
{
  elapsed[i] = 0;
  count[i] = 0;
}

float read_timer(int i)
{
  return elapsed[i] / 1000;
}

int read_count(int i)
{
  return count[i];
}
