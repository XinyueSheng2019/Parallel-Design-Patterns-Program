#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "pool.h"
#include "ran2.h"
#include <math.h>
#include <time.h>
#include "simulation.h"
#include "squirrel-actor.h"
#include <unistd.h>




int squirrel_actor(float x,float y, int infected);
void squirrel_stall(double delay);


int main(int argc, char* argv[])
{

    MPI_Init(&argc, &argv);
    int size, myrank;

    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
 

    if(size < NUM_OF_CELLS+MAX_SQUIRRELS+2){
       if(myrank == 0) printf("The size of processes is not enough. It should at least be %d.\n",(NUM_OF_CELLS+MAX_SQUIRRELS+2));
        MPI_Finalize();
        return 0;
    }

    int statuscode = processPoolInit();
    if (statuscode == 2){ //master actor

        //start a control actor
        int workerPid = startWorkerProcess();
        int actor = CONTROL_ACTOR;
        MPI_Ssend(&actor,1,MPI_INT,workerPid,0,MPI_COMM_WORLD);


    while(masterPoll()){}
    
    printf("SIMULATION ENDS.\n");

    }else if(statuscode == 1) {
        // worker processes
        int actor;
        MPI_Status status;
        float location[2];
       
        int workerstatus = 1;
        while(workerstatus){

            MPI_Recv(&actor,1,MPI_INT,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&status);
            
            if (actor == CONTROL_ACTOR){
                 workerstatus =control_actor();
            }else if (actor == CELL_ACTOR){
                 workerstatus = cell_actor();
            }else if (actor == SQUIRREL_ACTOR){
                float x = 0.0, y = 0.0;
                workerstatus = squirrel_actor(x,y,0);

            }
            else if (actor == NEW_SQUIRREL_ACTOR){
                
                MPI_Recv(&location,8,MPI_FLOAT,status.MPI_SOURCE,NEW_SQUIRREL_TAG,MPI_COMM_WORLD,&status);
                float x = location[0];
                float y = location[1];

                workerstatus = squirrel_actor(x,y,0);  

            }else if (actor == INFECTED_SQUIRREL_ACTOR){
                float x = 0.0, y = 0.0;
                workerstatus = squirrel_actor(x,y,1);
            }
            
        }
           
    }
    
  
    processPoolFinalise();
    MPI_Finalize();   
    return 0;

}

