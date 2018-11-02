#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "sll.h"
#include "queue.h"

/**
 * Queue data structure using local DLL class
 * 
 * @author Harry Bos
 */

//Define queue struct
typedef struct queue{
        SLL *sll;
}QUEUE;

QUEUE *newQUEUE(){
        QUEUE *queue = malloc(sizeof(QUEUE));
        queue->sll = newSLL();
        return queue;
}

void enqueue(QUEUE *items, void *value){
        insertSLL(items->sll, sizeSLL(items->sll), value);
}

void *dequeue(QUEUE *items){
        return removeSLL(items->sll, 0);
}

void *peekQUEUE(QUEUE *items){
        return getSLL(items->sll, 0);
}
int sizeQUEUE(QUEUE *items){
        return sizeSLL(items->sll);
}
