
//-------------------------------------------------
// File: producerconsumertest.c 
// Use:  producer consumer routine to test 
//       user thread lib and user semaphore. 
// @author: Mohammed Almarakby, Eslam Mohammed.
//-----------------------------------------------

#include "syscall.h"

#ifndef SEMT_H
#define SEMT_H
typedef int sem_t;
#endif

#define buffer_size 10
#define nb_producer buffer_size

static sem_t semLock2;
static sem_t semProduce;
static sem_t semConsume;

static int buffer[buffer_size];
static int consumerCounter = 0;
static int produceCounter = 0;
static int seed = 7;

//-----------------------------------------------
//producerRoutine: The usual Producer routine,
//takes an integer element and insert it in 
//buffer  
//-----------------------------------------------

void producerRoutine(int value){

    UserSemaphoreProberen(semProduce);
    UserSemaphoreProberen(semLock2);

    buffer[produceCounter] = value+seed *5;
    produceCounter = (produceCounter+1) % buffer_size;
    
    UserSemaphoreVerhogen(semLock2);
    UserSemaphoreVerhogen(semConsume);

}
//-----------------------------------------------
//consumerRoutine: The usual Consumer routine,
//takes an element from the buffer and display on 
//to console
//-----------------------------------------------

void consumerRoutine(void* NULL){

    int consumedElement;

    UserSemaphoreProberen(semConsume);
    UserSemaphoreProberen(semLock2);

    consumedElement = buffer[consumerCounter];
    consumerCounter = (consumerCounter+1) % buffer_size;
    PutString("consumed element: ");
    PutInt(consumedElement);
    PutChar('\n');

    UserSemaphoreVerhogen(semLock2);
    UserSemaphoreVerhogen(semProduce);

}

//-----------------------------------------------
//testConsumerProducer: The main routne to initiate
//the producer consumer
//-----------------------------------------------

void testConsumerProducer(){

    int i;
    semLock2 =  UserSemaphoreCreate("test lock semaphore", 1);
    semProduce =  UserSemaphoreCreate("test lock semaphore",nb_producer);
    semConsume =  UserSemaphoreCreate("test lock semaphore",0);
    
    for (i=0;i<nb_producer;i++){
        UserthreadCreate((void*) producerRoutine,(void*) i);
        UserthreadCreate((void*) consumerRoutine,0);
    }


}


int main(){
    
    testConsumerProducer();

    //TODO: destroying the sems automatically before halt routine 
    // or when the process finishes
    Halt();
    return 0;
}