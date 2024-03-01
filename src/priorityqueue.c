/**
 * @file priorityqueue.c
 * @brief Implementation of a priority queue.
 * @author Connor Johst - cjohst & Aaditya Suri - AadityaSuri
 * @bug No known bugs
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "priorityqueue.h"

/**
 * @brief Creates a priority queue.
 * 
 * @return A pointer to the newly created priority queue.
 */

PriorityQueue* createPriorityQueue(){
    PriorityQueue* priority_queue = (PriorityQueue*) malloc(sizeof(PriorityQueue));
    priority_queue->size = 0;
    return priority_queue;
}

/**
 * @brief Swaps two queue nodes.
 * 
 * @param a Pointer to the first queue node.
 * @param b Pointer to the second queue node.
 */

void swapQueueNodes(QueueNode* a, QueueNode* b){
    QueueNode temp = *a;
    *a = *b;
    *b = temp;
}

/**
 * @brief Heapifies the subtree rooted at the given node.
 * 
 * @param priority_queue The priority queue.
 * @param root The index of the root of the subtree to heapify.
 */

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

/**
 * @brief Enqueues a new element with the given priority into the priority queue.
 * 
 * @param priority_queue The priority queue.
 * @param priority The priority of the element to enqueue.
 * @param data The data associated with the element.
 * @return -1 if the queue is full, otherwise 0.
 */

int enqueue(PriorityQueue* priority_queue, int priority, char* data){
    if (priority_queue->size == MAX_QUEUE_SIZE){
        return -1;
    }

    QueueNode node_to_enqueue;

    node_to_enqueue.priority = priority;

    if (data){
        for (size_t n = 0; n < sizeof(data); n++) {
            node_to_enqueue.data[n] = data[n];
        }
    }

    int i = priority_queue->size + 1;
    priority_queue->size = priority_queue->size + 1;

    priority_queue->heap[i] = node_to_enqueue;


    while(i !=0 && priority_queue->heap[(i-1)/2].priority > priority_queue->heap[i].priority) {
        swapQueueNodes(&priority_queue->heap[i], &priority_queue->heap[(i-1)/2]);
        i = (i-1)/2;
    }

    return 0;

}

/**
 * @brief Dequeues the element with the highest priority from the priority queue.
 * 
 * @param priority_queue The priority queue.
 * @return The dequeued node.
 */

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

/**
 * @brief Returns the priority of the element with the highest priority in the priority queue.
 * 
 * @param priority_queue The priority queue.
 * @return The priority of the element at the front of the queue, or -1 if the queue is empty.
 */

int peak(PriorityQueue* priority_queue){
    if (priority_queue->size == 0){
        return -1;
    }
    return priority_queue->heap[0].priority;

}