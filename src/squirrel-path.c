#include "squirrel-path.h"


/**
 * This is a squirrel's path. It is designed for storing populationinflux and infectionlvel for each step.
 * The max step is 50. It will pop the earlist step value firstly and push the new step value when the queue is full.
 * In this way, the wasted storage space can be avoided and it is easy to pop and push elements.
 */

//init the queue
void queue_init(Queue * p_queue)
{
	p_queue->head = 0;
	p_queue->tail = 0;
	int n = 0;
	for(n = 0;n<SIZE;n++)
	{
		p_queue->arr[n] = 0;
	}
}

//clear the queue
void queue_deinit(Queue *p_queue){
	p_queue->head = 0;
	p_queue->head = 0;
}

//calculate the size 
int queue_size(const Queue *p_queue){
	return (p_queue->tail - p_queue->head);
}

//judge whether the queue is empty
int queue_empty(const Queue *p_queue) {
	return !(p_queue->tail - p_queue->head);
}

//judge whether the queue is full
int queue_full(const Queue *p_queue) {
	return p_queue->tail >= SIZE;
}

//push element
int queue_push(Queue *p_queue, int val) {
	if (queue_full(p_queue)) {
		return 0;
	}
	else {
		p_queue->arr[p_queue->tail] = val;
		p_queue->tail++;
		return 1;
	}
}

//pop the earlist stored element
int queue_pop(Queue *p_queue) {
	if (queue_empty(p_queue)) {
		return 0;
	}
	else {
		int n = 0;
		for(n=0;n<SIZE-1;n++){
			p_queue->arr[n] = p_queue->arr[n+1];
		}
		p_queue->arr[SIZE-1] = 0;
		p_queue->tail --;

		return 1;
	}
}

//print the trajectory
void queue_print(Queue *p_queue){
	printf("the queue is :");
	int n = 0;
	for(n=0;n<queue_size(p_queue);n++){
		printf("%d  ", p_queue->arr[n]);
	}
	printf("\n");
}

//calculate the mean of all elements in the queue
float queue_mean(Queue *p_queue) {
    if (queue_empty(p_queue)) {
		return 0;
	}
    else{
        float sum_value = 0.0;
        int n=0;
        
        for(n=0;n<queue_size(p_queue);n++){
            sum_value += p_queue->arr[n]; 
        }
        return sum_value/queue_size(p_queue);


    }
}




