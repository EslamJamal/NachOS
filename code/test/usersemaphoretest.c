

//-------------------------------------------------
// File: usersemaphoretest.c 
// Use:  routine to test user level semaphore      
// @author: Mohammed Almarakby, Eslam Mohammed.
//-----------------------------------------------


#include "syscall.h"

#ifndef SEMT_H
#define SEMT_H
typedef int sem_t;
#endif

#define buffer_size 10
#define nb_producer buffer_size
static sem_t semLock1;

//-----------------------------------------------
//TestPrint: A locked routine to print strings on the 
//           on the console. 
//-----------------------------------------------

void testPrint(int id){

    UserSemaphoreProberen(semLock1);
    PutString("thread Id: ");
    PutInt(id);
    PutChar('\n');
    PutString("Writing characters to the console\n");
    PutChar('a');
    PutChar('b');
    PutChar('c');
    PutChar('\n');
    PutString("writing number to the console\n");
    PutInt(1);
    PutInt(2);
    PutInt(3);
    PutChar('\n');
    UserSemaphoreVerhogen(semLock1);

}

//-----------------------------------------------
// testSemaphore: The main routine to  test the 
// user level semaphore locking 
//-----------------------------------------------

void testSemaphore(){
   
    semLock1 =  UserSemaphoreCreate("test lock semaphore",1);
    PutString("semaphore Id: ");
    PutInt((int)semLock1);
    PutChar('\n');
    PutString("initiating multiple threads\n");
    UserthreadCreate((void*) testPrint,(void*) 0 );
    UserthreadCreate((void*) testPrint,(void*) 1 );
    UserthreadCreate((void*) testPrint,(void*) 2 );
}



int main(){

        testSemaphore();
        Halt();

}