#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "control-actor.h"
#include "simulation.h"
#include <assert.h>

int control_actor(){

    MPI_Send(NULL, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);

    int i, j, workerPid, actor, size;
    int current_squirrels=0, infected_squirrels=0, month_dead=0, month_born=0;
    float born_rate=0.0, death_rate=0.0, infected_rate=0.0;
    int current_tag=0, terminated=0, max_tag=0;
    float location[2];

    int month_id = 1;
    double begin;
    double time_spent;
    int cell_rank;
    int cell_msg[NUM_OF_CELLS][2];

    MPI_Request clock_request[NUM_OF_CELLS], cell_request[NUM_OF_CELLS];
    MPI_Status status;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Datatype cell_msg_type;
    MPI_Type_vector(2,1,1,MPI_INT,&cell_msg_type);
    MPI_Type_commit(&cell_msg_type);

    init_actors(&current_squirrels,&infected_squirrels);

    // initial the cell_msg
    for (i=0;i<NUM_OF_CELLS;i++){ for(j=0;j<2;j++){cell_msg[i][j] = 0;}}

    print_start_signal();

    /**
     * This is the clock for the simulation, the control actor will keep receiving and sending message from/to squirrel actors and cell actors.
     */
    for (month_id = 0; month_id < MODEL_MONTH; month_id ++){ 
        
        begin = MPI_Wtime(); 

        terminated = judge_squirrel_num(current_squirrels, infected_squirrels);
        if (terminated) break;

        while(MPI_Wtime() - begin < TIMEGAP){
            //receive message from squirrels
            MPI_Iprobe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&current_tag,&status);
            if(current_tag)
            {
                if(status.MPI_TAG == NEW_SQUIRREL_TAG)
                {
                    //received new squirrel
                    MPI_Recv(NULL,0,MPI_INT,status.MPI_SOURCE,NEW_SQUIRREL_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE); 
                    if(current_squirrels==MAX_SQUIRRELS) max_tag = 1;
                    MPI_Bsend(&max_tag,1,MPI_INT,status.MPI_SOURCE,VALID_BORN_TAG,MPI_COMM_WORLD);
                    if (max_tag) break; 
                    current_squirrels ++;
                    month_born ++;
            
                }else if (status.MPI_TAG ==SQUIRREL_DIE_TAG )
                {
                    //receive dead squirrel
                    MPI_Recv(NULL,0,MPI_INT,status.MPI_SOURCE,SQUIRREL_DIE_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                    current_squirrels --;
                    infected_squirrels --;
                    month_dead ++;
            
                }
                else if(status.MPI_TAG == SQUIRREL_INDECTED_TAG)
                {
                    //receive infected squirrel
                    MPI_Recv(NULL,0,MPI_INT,status.MPI_SOURCE,SQUIRREL_INDECTED_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                    infected_squirrels++;
            
                }
            }
            }
            //alert cells the month changes, and receive cells' updated population influx and infection level
            for (cell_rank=2;cell_rank<2+NUM_OF_CELLS;cell_rank++)
            {
                MPI_Isend(&month_id,1,MPI_INT,cell_rank,MONTH_ALERT_TAG,MPI_COMM_WORLD,&clock_request[cell_rank]);
                MPI_Irecv(&cell_msg[cell_rank-2][0],1,cell_msg_type,cell_rank, CELL_UPDATE_TAG,MPI_COMM_WORLD,&cell_request[cell_rank]);
            } 
            MPI_Waitall(NUM_OF_CELLS,cell_request,MPI_STATUS_IGNORE);
            
            display_cell_info(month_id, cell_msg,current_squirrels,infected_squirrels,month_dead,month_born);

            month_dead = 0;
            month_born = 0;
         
    }

    shutdownPool();
   
    return workerSleep();
  
    
}

/**
 * This function is used for display the cell's information for each month
 */
void display_cell_info(int month_id, int cell_msg[NUM_OF_CELLS][2], int current_squirrels, int infected_squirrels, int month_death, int month_born)
{
    int i,j;
     printf("MONTH: %d\t ALIVE: %d \t INFECTED: %d \t HEALTHY: %d \t LAST MONTH DEATH: %d \t LAST MONTH BORN: %d\n", month_id+1, current_squirrels, infected_squirrels, current_squirrels-infected_squirrels,month_death, month_born);
    printf("-----------------------------------------------------------------------------------------------------------------------------------------------------\n");
    printf("CellId  \t\t");
    for(i=0;i<NUM_OF_CELLS;i++)
    {
        printf("%d\t",i);
    }
    printf("\n");
    for (j=0;j<2;j++)
    {
        if(j==0)
        {
            printf("PopulationInflux  \t");
            for(i=0;i<NUM_OF_CELLS;i++)
            {
                printf("%d\t",cell_msg[i][0]);
            }
        }
        if(j==1)
        {
            printf("\nInfectionLevel  \t");
            for(i=0;i<NUM_OF_CELLS;i++)
            {
                printf("%d\t",cell_msg[i][1]);
            }
            printf("\n");
        }       
    }
    printf("-----------------------------------------------------------------------------------------------------------------------------------------------------\n");

}

/**
 * Print to the screen: simulation start.
 */
void print_start_signal()
{
    printf("SIMULATION STARTS.\n-----------------------------------------------------------------------------------------------------------------------------------------------------\n");
}


/**
 * Judge the valid of current squirrel number.
 * if it is over max squirrel number, the program will appear error.
 * if it is 0, it means all squirrel have die, the program need to end.
 */
int judge_squirrel_num(int current_squirrels, int infected_squirrels)
{
     if (current_squirrels>=MAX_SQUIRRELS)
     {
        printf("ERROR: The number of alive squirrels is more than 200.\n");
        assert(current_squirrels<MAX_SQUIRRELS);
       
        return 1;
    } 
    else if (current_squirrels<=0 && infected_squirrels==0){
        printf("ALL SQUIRRELS DIED.\n");
        return 1;
    }
    else{
        return 0;
    }

}


/**
 * Initial all actors: healthy squirrels, infected squirrels and cells
 */
void init_actors(int *current_squirrels, int *infected_squirrels)
{
    //set send buffer
    int buffer_attached_size;
    char * buffer_attached;
    buffer_attached_size = MPI_BSEND_OVERHEAD + sizeof(int)*(NUM_OF_CELLS+INIT_SQUIRRELS+INIT_INFECTED);
    buffer_attached = (char*)malloc(buffer_attached_size);
    MPI_Buffer_attach(buffer_attached, buffer_attached_size);
    
    int i, workerPid, actor;

    //init cell_actor
   for(i=0;i<NUM_OF_CELLS;i++){
       workerPid = startWorkerProcess();
       actor = CELL_ACTOR;
       MPI_Bsend(&actor,1,MPI_INT,workerPid,0,MPI_COMM_WORLD);
   }

   //init squirrel_actor
   for(i=0;i<INIT_SQUIRRELS;i++){
       workerPid = startWorkerProcess();
       actor = SQUIRREL_ACTOR;
       MPI_Bsend(&actor,1,MPI_INT,workerPid,0,MPI_COMM_WORLD);
       *current_squirrels = *current_squirrels + 1;
    
   }

   //init infected squirrel_actor
   for(i=0;i<INIT_INFECTED;i++){
       workerPid = startWorkerProcess();
       actor = INFECTED_SQUIRREL_ACTOR;
       MPI_Bsend(&actor,1,MPI_INT,workerPid,0,MPI_COMM_WORLD);
       *current_squirrels = *current_squirrels + 1;
       *infected_squirrels = *infected_squirrels + 1;
   }
    MPI_Buffer_detach(&buffer_attached,&buffer_attached_size);
    free(buffer_attached);
}




