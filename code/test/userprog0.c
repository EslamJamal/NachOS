

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
        for(j=0;j<i;j++){
            PutChar('.');
        }
            
            PutInt(i*10);
            PutChar('%');
            PutChar('\n');
    }
    PutChar('\n');
    UserSemaphoreVerhogen(semLock1);

}

void thread1(void* arg){

    PutString("Thread routine starts here\n");
    PutChar('0');
    PutChar('0');
    PutChar('0');
    PutChar('\n');
    PutString("thread routine ended\n");

}


void thread2(void* arg){

    PutString("Thread routine starts here\n");
    PutChar('1');
    PutChar('1');
    PutChar('1');
    PutChar('\n');
    PutString("thread routine ended");


}


int main(){

    semLock1 =  UserSemaphoreCreate("test lock semaphore",1);
    PutString("starting forked process routine\n");
    
    UserthreadCreate((void*) thread1,(void*) 0); 
    UserthreadCreate((void*) thread2,(void*) 0); 
    // PutString("ending forked process routine\n");


    ProcessTerminate();


    /* not reached */
    return 0;

}
