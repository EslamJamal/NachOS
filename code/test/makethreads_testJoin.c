//-------------------------------------------------
// File: makethreads_testJoin.c 
// Use: user program to test user thread join 
// @author: Mohammed Almarakby, Eslam Mohammed.
//-------------------------------------------------

#include "syscall.h"
static sem_t semLock1;

int i, j;

//-------------------------------------------------
// DummyRoutine: A routine for testing user threads
// ability to do calculations and basic system call. 
//-------------------------------------------------

void DummyRoutine(){

    UserSemaphoreProberen(semLock1);
    for(i=0;i<11;i++){
        for( j=0;j<i;j++){
            PutChar('.');
        }
            
            PutInt(i*10);
            PutChar('%');
            PutChar('\n');
    }
    PutChar('\n');
    UserSemaphoreVerhogen(semLock1);

}

void thread1(void * NULL){

  DummyRoutine();
}

void thread2(void* NULL){

    
    UserThreadJoin(0); // if this thread id is 1, it will wait on itself ?
    DummyRoutine();    
}

void thread3(void* NULL){

    UserThreadJoin(1); // if this thread id is 1, it will wait on itself ?
    DummyRoutine();    


}


int main(){

    semLock1 =  UserSemaphoreCreate("test lock semaphore",1);

    UserthreadCreate((void*) thread1, (void*) 0);
    UserthreadCreate((void*) thread2, (void*) 0);
    UserthreadCreate((void*) thread1, (void*) 0);




    Halt();
    
    return 0;
}



