#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "simulation.h"
#include "cell-actor.h"
#include "pool.h"


int cell_actor(){
    /*
    This is cell actor, 
    1. the cell always receives the month change alerts from the control actor and update its populationInflux and infectionLevel.
    2. When a squirrel steps into this cell, it records this visit, and provides the updated cell information.
    3. When a squirrel dies in this cell, the virus will remain in this cell for two month, and the cell infectionLevel for this month and next month will update(+1).
    */

    int myrank, tag;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Request request;
    MPI_Status start_status,status;

    //calculation variables
    int month_populationInflux[MODEL_MONTH], month_infectionlevel[MODEL_MONTH];
    int cell_msg[2];
    int populationInflux=0, infectionlevel=0;
    int month = 0;
    int infected;

    //Bsend buffer
    int buffer_attached_size;
    char * buffer_attached;
    buffer_attached_size = MPI_BSEND_OVERHEAD + sizeof(int)*100;
    buffer_attached = (char*)malloc(buffer_attached_size);
    MPI_Buffer_attach(buffer_attached, buffer_attached_size);
    
    //create and init cell_msg
    cell_msg[0] = 0;
    cell_msg[1] = 0;
    int i;
    for(i=0;i<MODEL_MONTH;i++){
        month_infectionlevel[i] = 0;
        month_populationInflux[i] = 0;
    }
   
      
    //receive the the first month signal from the control actor before the loop.
    MPI_Recv(&month, 1, MPI_INT, CONTROL_ACTOR, MONTH_ALERT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    cell_msg[0] = month_populationInflux[0];
    cell_msg[1] = month_infectionlevel[0];

    //send the first month cell's population and infection level to the control actor 
    MPI_Bsend(cell_msg,2,MPI_INT,CONTROL_ACTOR,CELL_UPDATE_TAG,MPI_COMM_WORLD);

    while(1)
    {     
        //judge whether it is asked to terminate
        if (shouldWorkerStop()) break;
      
        MPI_Iprobe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&tag,&status);
        if(tag){
            if(status.MPI_TAG == MONTH_ALERT_TAG)
            {
            //update new month populationinflux and infectionlevel
            MPI_Recv(&month, 1, MPI_INT, CONTROL_ACTOR, MONTH_ALERT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            update_cell_info(&cell_msg[0], month_populationInflux, month_infectionlevel, month);
            //send the updated information to the control actor
            MPI_Bsend(cell_msg,2,MPI_INT,CONTROL_ACTOR,CELL_UPDATE_TAG,MPI_COMM_WORLD);
            if(CELL_DEBUG)
            {
                if (myrank == 2) print_cell_month_info((myrank-2), month, month_populationInflux, month_infectionlevel);
            } 

            }
            else if(status.MPI_TAG == STEP_INTO_CELL_TAG)
            {
            //when a squirrel steps into this cell, the current month's population and infectionlevel updates.
            //Then, the cell sends this month's cell information(updated per month) to the squirrel.
            MPI_Recv(&infected,1,MPI_INT,status.MPI_SOURCE,STEP_INTO_CELL_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            month_populationInflux[month] += 1;
            month_infectionlevel[month] += infected; 
            MPI_Bsend(&cell_msg,2,MPI_INT,status.MPI_SOURCE,CELL_MSG_TAG,MPI_COMM_WORLD);
            }
            else if(status.MPI_TAG == SQUIRREL_DIE_TAG)
            {
            //when a squirrel is going to die, it sends message to the cell it dies in.
            //The virus will remain in this cell for 2 months.
            MPI_Recv(NULL,0,MPI_INT,status.MPI_SOURCE,SQUIRREL_DIE_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            if (month!=23) month_infectionlevel[month + 1] += 1;
            }
        }
        
        
    }
    MPI_Buffer_detach(&buffer_attached,&buffer_attached_size);
    free(buffer_attached);
    MPI_Send(NULL, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
    
    return workerSleep();

   
}

void update_cell_info(int* cell_msg, int month_populationInflux[MODEL_MONTH], int month_infectionlevel[MODEL_MONTH], int month)
{ 
    /*
    update the cell's populationInflux and infectionlevel when the month changes.
    */
    
    int populationInflux=0,infectionlevel=0;

    if(month>2){  
        populationInflux = month_populationInflux[month-1] + month_populationInflux[month-2] + month_populationInflux[month-3];
        infectionlevel = month_infectionlevel[month-1] + month_infectionlevel[month-2];
    }else if(month==2){
        populationInflux = month_populationInflux[1] + month_populationInflux[0];
        infectionlevel = month_infectionlevel[1] + month_infectionlevel[0];
    }else{
        populationInflux = month_populationInflux[0];
        infectionlevel = month_infectionlevel[0];
    }
        cell_msg[0] = populationInflux;
        cell_msg[1] = infectionlevel;
                
}


/**
 * to test the correctness of the simulation, print the single month's populationInflux and infectionlevel
 */
void print_cell_month_info(int id, int month, int month_populationInflux[MODEL_MONTH], int month_infectionlevel[MODEL_MONTH])
{
    printf("cell %d month %d month_populationInflux = %d, month_infectionlevel = %d \n", id, month, month_populationInflux[month-1], month_infectionlevel[month-1]);
}