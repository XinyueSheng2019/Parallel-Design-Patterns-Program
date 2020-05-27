#include "squirrel-path.h"

#define SQUIRREL_DELAY 0.0

int squirrel_actor(float x,float y, int infected);
void squirrel_stall(double delay);
int test_infection(int *infected, float mean_infection, long state);
int test_death(long state, int target_cell);
void send_death_msg(int target_cell);
int test_born(float mean_population, long state, float x, float y);
int allocate_new_squirrel(float x, float y);