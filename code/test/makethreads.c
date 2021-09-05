
//-------------------------------------------------
// File: makethreads.c 
// Use: user program to test the basic
//      userthread lib routines 
// @author: Mohammed Almarakby, Eslam Mohammed.
//-------------------------------------------------


#include "syscall.h"

static sem_t semLock1;




//-------------------------------------------------
// DummyRoutine: A routine for testing user threads
// ability to do calculations and basic system call. 
//-------------------------------------------------

void DummyRoutine(){
    int i, j;
    for(i=0;i<11;i++){
        for(j=0;j<i;j++){
            PutChar('.');
        }
            
            PutInt(i*10);
            PutChar('%');
            PutChar('\n');
    }
    PutChar('\n');

}

//-------------------------------------------------
//PutToConsoleTest: The main routine for the user
// thread testing.
//-------------------------------------------------

void PutToConsoleTest(char arg){

    UserSemaphoreProberen(semLock1);
    PutString("User thread id: ");
    PutChar(arg);
    PutString(" starts the routine\n");
    DummyRoutine();    
    PutString("end of user thread routine\n");

    UserSemaphoreVerhogen(semLock1);
    // UserThreadExit(); //Termination is automatic now

}

void readAndWriteTest(void* arg){
    //TODO: complete the test 
    char c[2] ;   
    GetString(c,2);
    PutString("Writing string....\n");
    PutChar('\n');
    // UserThreadExit(); //Termination is automatic now

}


int main(){

    PutString("main routine started\n");

    //User level semaphore to lock the dummyPause routine.
    semLock1 =  UserSemaphoreCreate("test lock semaphore",1);

    PutString("initiating user threads\n");
    UserthreadCreate((void*) PutToConsoleTest,(void*) '1');
    // UserthreadCreate((void*) PutToConsoleTest,(void*) '2');
    // UserthreadCreate((void*) PutToConsoleTest,(void*) '3');
    // UserthreadCreate((void*) readAndWriteTest,(void*) 0);

    Halt();// Halt is not automatic for now
    /* not reached */
    return 0; 
    
}

