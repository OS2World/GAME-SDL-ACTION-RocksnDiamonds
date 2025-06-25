#include "random_gen.h"
#include <time.h>
#include <stdlib.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#define RANDOM_LIST_SIZE (1L<<18)
static float random_list[RANDOM_LIST_SIZE];
static unsigned long random_list_index;
static unsigned long new_random_index = 0;

/*
 * Random numbers involve a call to the operating system, and may take some
 * time to complete. I can't call the random number generator multiple times
 * per frame.  In one old version, it would take half a second before the ship
 * explosion was generated when the rnd() function was calling the operating
 * system function.
 * The solution is to generate random numbers during initialisation, and store
 * in a circular array.  This will (of course) repeat, but it is not noticeable
 * (unless RANDOM_LIST_SIZE is very very small).
 */

float rnd_next_float() {
  return (float) random() / (float) RAND_MAX;
}

void initrnd() {
  unsigned long i;
  time_t starttime;

  if(time(&starttime) != -1) {
    srandom((unsigned int) starttime);
  }
  for(i = 0; i < RANDOM_LIST_SIZE; i++) {
    random_list[i] = rnd_next_float();
  }
  random_list_index = 0;
#ifdef DEBUG
  float min = *random_list;
  float max = *random_list;
  for(i = 0; i < RANDOM_LIST_SIZE; ++i) {
    float next = random_list[i];
    if(next < min) min = next;
    if(next > max) max = next;
  }
  printf("%ld random numbers have been initialised (min=%g, max=%g).\n", RANDOM_LIST_SIZE, min, max);
#endif
}


float rnd() {
  if(++random_list_index >= RANDOM_LIST_SIZE) {
    random_list_index = 0;
    //From time to time insert a new random number to keep the list "fresh".
    random_list[new_random_index++ % RANDOM_LIST_SIZE] = rnd_next_float();
#ifdef DEBUG
    puts("Random numbers are recycled!");
#endif
  }
  return random_list[random_list_index];
}
