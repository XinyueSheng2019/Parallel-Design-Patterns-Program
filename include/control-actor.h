#include "simulation.h"
#include "mpi.h"
int control_actor();
void init_actors(int *current_squirrels, int *infected_squirrels);
void print_start_signal();
void display_cell_info(int month_id, int cell_msg[NUM_OF_CELLS][2], int current_squirrels, int infected_squirrels, int month_death, int month_born);
int judge_squirrel_num(int current_squirrels, int infected_squirrels);
