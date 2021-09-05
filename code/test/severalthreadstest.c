//-------------------------------------------------
// File: routine to test large number of threads,
//		 Cotinuously create threads to jam the
//		 the system and test its handling.
// @author: Mohammed Almarakby, Eslam Mohammed
//-------------------------------------------------

#include "syscall.h"
#define NB_THREADS 10



static sem_t semLock1;


//-------------------------------------------------
// DummyRoutine: A routine for testing user threads
// ability to do calculations and basic system call. 
//-------------------------------------------------

void DummyRoutine(){
	int i, j;
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


//-------------------------------------------------
//PutToConsoleTest: The main routine for the user
// thread testing.
//-------------------------------------------------

void threadRoutine(char args){
	
	DummyRoutine();
	// UserThreadExit();  //Termination is automatic now

}


//-------------------------------------------------
//thredBomd: recursively create threads.
//-------------------------------------------------


void threadBomb(void* NULL){

	PutChar('.');
	PutChar('\n');
	UserthreadCreate((void*) threadBomb,0); 
}




int main(){


    semLock1 =  UserSemaphoreCreate("test lock semaphore",1);
	int i;

	for ( i =0;i<NB_THREADS;i++){
		UserthreadCreate((void*) threadRoutine,(void*) 0); 
	}

	// UserthreadCreate((void*) threadBomb,(void*) 0); 
	
	Halt();
	/* not reached */
    return 0;
}
