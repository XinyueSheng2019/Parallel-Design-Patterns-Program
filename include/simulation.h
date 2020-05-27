
#define NUM_OF_CELLS 16
#define INIT_SQUIRRELS 34
#define MAX_SQUIRRELS 200
#define INIT_INFECTED 4
#define MODEL_MONTH 24
#define TIMEGAP 0.5

#define NEW_SQUIRREL_TAG 99999 //everytime a new squirrel is born, the parent sends a msg to the control actor
#define SQUIRREL_DIE_TAG 99998
#define SQUIRREL_INDECTED_TAG 99990
#define MONTH_ALERT_TAG 99997
#define STEP_INTO_CELL_TAG 99995 //everytime a squirrel steps into a cell, the squirrel sends a msg to the cell
#define CELL_MSG_TAG 99994
#define CELL_UPDATE_TAG 99989
#define VALID_BORN_TAG 99993

#define MASTER 0
#define CONTROL_ACTOR 1
#define CELL_ACTOR 2
#define SQUIRREL_ACTOR 3
#define NEW_SQUIRREL_ACTOR 4
#define INFECTED_SQUIRREL_ACTOR 5

#define CELL_DEBUG 0 //test the correctness of this simulation

#define CN 4  //change the number of cells, if CN=4, it means 16 cells; if CN=5, it means 25 cells.