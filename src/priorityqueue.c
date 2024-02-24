#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "priorityqueue.h"


PriorityQueue* createPriorityQueue(){
    PriorityQueue* priority_queue = (PriorityQueue*) malloc(sizeof(PriorityQueue));
    priority_queue->size = 0;
    return priority_queue;
}

void swapQueueNodes(QueueNode* a, QueueNode* b){
    QueueNode temp = *a;
    *a = *b;
    *b = temp;
}

//heapify subtree rooted at node root 

void heapify(PriorityQueue* priority_queue, int root){
    int smallest = root;
    int left = 2 * root + 1;
    int right = 2 * root + 2;

    if (left < priority_queue->size){
        if (priority_queue->heap[left].priority < priority_queue->heap[smallest].priority) {
            smallest = left;
        }
    }

    if (right < priority_queue->size) {
        if (priority_queue->heap[right].priority < priority_queue->heap[smallest].priority){
            smallest = right;
        }
    }

    if (smallest != root) {
        swapQueueNodes(&priority_queue->heap[root], &priority_queue->heap[smallest]);
        heapify(priority_queue, smallest);
    }

}

int enqueue(PriorityQueue* priority_queue, int priority, packet_t packet){
    if (priority_queue->size == MAX_QUEUE_SIZE){
        return -1;
    }

    int i = priority_queue->size + 1;
    priority_queue->size = priority_queue->size + 1;

    priority_queue->heap[i].priority = priority;
    priority_queue->heap[i].packet = packet;

    while(i !=0 && priority_queue->heap[(i-1)/2].priority > priority_queue->heap[i].priority) {
        swapQueueNodes(&priority_queue->heap[i], &priority_queue->heap[(i-1)/2]);
        i = (i-1)/2;
    }

}

QueueNode dequeue(PriorityQueue* priority_queue){
    if (priority_queue->size == 0){
        fprintf(stderr, "ERROR: Dequeue from empty priority queue\n");
        exit(EXIT_FAILURE);
    }

    if (priority_queue->size == 1){
        priority_queue->size--;
        return priority_queue->heap[0];
    }

    QueueNode root = priority_queue->heap[0];
    priority_queue->heap[0] = priority_queue->heap[priority_queue->size-1];
    priority_queue->size--;
    heapify(priority_queue,0);

    return root;

}

int peak(PriorityQueue* priority_queue){
    if (priority_queue->size == 0){
        return -1;
    }
    return priority_queue->heap[0].priority;

}