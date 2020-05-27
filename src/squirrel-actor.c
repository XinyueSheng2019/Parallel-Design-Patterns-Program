#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "pool.h"
#include "simulation.h"
#include "squirrel-path.h"
#include "squirrel-actor.h"
#include "squirrel-functions.h"
#include <time.h>


 /**
  * This is squirrel actor.
  */
int squirrel_actor(float x,float y, int infected)
{
   
    int myrank, end_tag=0;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Status status;

    long state = -1-myrank;
    int cell_msg[2];
    int birth = 0, die = 0, step = 0, infected_step = 0;
    int target_cell;
    float mean_population, mean_infection;
    int terminated;

    //create 2 trajectory arrays storing step_population and step_infection seperately for consecutive 50 steps
    Queue step_populationinflux;
    Queue step_infectionlevel;
    queue_init(&step_populationinflux);
    queue_init(&step_infectionlevel);
    
    //Bsend buffer allocated
    int buffer_attached_size;
    char * buffer_attached;
    buffer_attached_size = MPI_BSEND_OVERHEAD + sizeof(int)*2000000;
    buffer_attached = (char*)malloc(buffer_attached_size);
    MPI_Buffer_attach(buffer_attached, buffer_attached_size);

    
    initialiseRNG(&state); //initial the squirrel's position
    
    while (1) 
    {
        //judge whether it is asked to terminate
        if (shouldWorkerStop()) break; 
      
        squirrel_stall(SQUIRREL_DELAY); 
        
        
        //step into a cell
        squirrelStep(x, y, &x, &y, &state);
        target_cell = getCellFromPosition(x,y) + 2;
       
        if (shouldWorkerStop()) break; 

        //send squirrel condition to the cell and receive the cell's populationinflux and infectionlevel
        MPI_Bsend(&infected,1,MPI_INT,target_cell,STEP_INTO_CELL_TAG,MPI_COMM_WORLD);
        MPI_Recv(&cell_msg,2,MPI_INT,target_cell,CELL_MSG_TAG,MPI_COMM_WORLD,&status);
        
        step ++;

        //judge whehter the squirrel has walked for over 50 steps, if its trajectories sizes are over 50, they should delete the earlist step's information.
        if(queue_full(&step_populationinflux)){
            queue_pop(&step_populationinflux);
            queue_pop(&step_infectionlevel);
        }
        //push cell_msg to the arrays
        queue_push(&step_populationinflux,cell_msg[0]);
        queue_push(&step_infectionlevel,cell_msg[1]);

        mean_infection = queue_mean(&step_infectionlevel);
        

        terminated = test_infection(&infected, mean_infection, state);
        if (terminated) break;
        
        
       
        // if the squirrel is infected, it will be tested death condition when it walks for over 50 steps
        if(infected == 1)
        {
            infected_step ++;
            if(infected_step>50) terminated = test_death(state, target_cell);
            if (terminated) break;   
        }

        // for each 50 steps, the swquirrel will be tested whehter it will born a new squirrel.
        if(step % 50 == 0) 
        {
            mean_population = queue_mean(&step_populationinflux);
            terminated = test_born(mean_population, state, x, y);
            if (terminated) break;
        }
                
    }
    MPI_Buffer_detach(&buffer_attached,&buffer_attached_size);
    free(buffer_attached);
  
    MPI_Send(NULL, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
    return workerSleep();
    
    
}

/**
 * adjust the squirrel move speed
 */
void squirrel_stall(double delay)
{   
    double begin = MPI_Wtime();
    double time_spent=0.0;
    while(time_spent<delay)
    {
        time_spent = MPI_Wtime()- begin;
    }

}

/**
 * test whether the squirrel is infected.
 */
int test_infection(int *infected, float mean_infection, long state )
{
    if(*infected == 0) {
        *infected = willCatchDisease(mean_infection,&state);
        if (*infected == 1){
            if (shouldWorkerStop()) return 1;

            MPI_Bsend(NULL,0,MPI_INT,CONTROL_ACTOR,SQUIRREL_INDECTED_TAG,MPI_COMM_WORLD);
            
        }
    }
    return 0;
}

/**
 * When the infected squirrel has walked over 50 steps, it will be tested whether it is going to die for each next step.
 */
int test_death(long state, int target_cell)
{      
        int die = willDie(&state);
        
        if(die == 1) 
        {  
            if (shouldWorkerStop()) 
            {
                return 1;
            }else{
                send_death_msg(target_cell);
                return 1;
            }
        }else{
            return 0;
        }     
    
}

/**
 * When a squirrel is about to die, it should send a message to control actor to reduce to reduce the current alive squirrels.
 * It also needs to tell the cell and the cell will record the virus for 2 months.
 */
void send_death_msg(int target_cell)
{
    MPI_Bsend(NULL,0,MPI_INT,CONTROL_ACTOR,SQUIRREL_DIE_TAG,MPI_COMM_WORLD);
    MPI_Bsend(NULL,0,MPI_INT,target_cell,SQUIRREL_DIE_TAG,MPI_COMM_WORLD);
}


/**
 * After each 50 steps, the squirrel will be tested whehter it will born new squirrel. 
 * if True, the new squirrel's process and location will be set.
 */
int test_born(float mean_population, long state, float x, float y)
{
    int birth = willGiveBirth(mean_population,&state);
    int error;
        
    if (birth==1)
    {
        if (shouldWorkerStop()) 
        {
            return 1;
        }else{
            error = allocate_new_squirrel(x, y); 
            if(error){
                return 1;
            }else{
                return 0;
            }
           
        }
    }else{
        return 0;
    }
    
}

/**
 * Allocate a new squirrel's process and send its location to its process.
 */
int allocate_new_squirrel(float x, float y)
{
    int location[2];
    location[0] = x;
    location[1] = y;
    int max_tag;
    
    
    MPI_Bsend(NULL,0,MPI_INT,CONTROL_ACTOR,NEW_SQUIRREL_TAG,MPI_COMM_WORLD); //send to control_actor to current_squirrel++
    MPI_Recv(&max_tag,1,MPI_INT,CONTROL_ACTOR,VALID_BORN_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    if(max_tag==0)
    {
        if(shouldWorkerStop()==0)
        {
            int workerPid = startWorkerProcess();
            int actor = NEW_SQUIRREL_ACTOR;
            MPI_Bsend(&actor,1,MPI_INT,workerPid,0,MPI_COMM_WORLD);
            MPI_Bsend(&location,8,MPI_FLOAT,workerPid,NEW_SQUIRREL_TAG,MPI_COMM_WORLD); 
            return 0;
        }else
        {
            return 1;
        }   
    }
    else
    {
        return 1;
    }
    
    

   


}