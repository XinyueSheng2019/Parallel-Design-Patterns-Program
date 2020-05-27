#ifndef  __QUEUE_1_H__
#define  __QUEUE_1_H__

#define SIZE 50


typedef struct {
	int arr[SIZE];
	int head; 
	int tail; 
}Queue;

void queue_init(Queue *p_queue);
void queue_deinit(Queue *p_queue);
int queue_size(const Queue *p_queue);
int queue_empty(const Queue *p_queue);
int queue_full(const Queue *p_queue);
int queue_push(Queue *p_queue, int val);
int queue_pop(Queue *p_queue);
float queue_mean(Queue *p_queue);
void queue_print(Queue *p_queue);

#endif  //QUEUE_1_H__