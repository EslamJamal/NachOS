
//-------------------------------------------------
// File: testforkexec: testing the spawning of 
//        multiple user processes with multiple 
//        threads inside.
// @author: Mohammed Almarakby, Eslam Mohammed
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


int main()
{
    semLock1 =  UserSemaphoreCreate("test lock semaphore",1);

    ForkExec("./userprog0");
    ForkExec("./userprog0");

    ForkExec("./userprog1");
    ForkExec("./userprog1");
    
    PutString("starting main process routine\n");
    // DummyRoutine();
    ProcessTerminate();
    /* not reached */
    return 0;
}
