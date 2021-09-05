
#include "syscall.h"

int i, j;

//-------------------------------------------------
// DummyRoutine: A routine for testing user threads
// ability to do calculations and basic system call. 
//-------------------------------------------------

void DummyRoutine(void * NULL){
    
    for( i=0;i<11;i++){
        for( j=0;j<i;j++){
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



int main(){

    PutString("main routine started\n");

    PutString("initiating user threads\n");
    UserthreadCreate((void*) DummyRoutine,(void*) 0);
    UserthreadCreate((void*) DummyRoutine,(void*) 0);

    Halt();// Halt is not automatic for now 
    /* not reached */
    return 0;
}

