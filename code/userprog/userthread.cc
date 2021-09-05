

//-------------------------------------------------
// File: userthread.cc 
// Use:  User thread routines to initiate, execute,
//       exit and join a user thread.
// @author: Mohammed Almarakby, Eslam Mohammed.
//-------------------------------------------------


#include "system.h"
#include "userthread.h"
#include "thread.h"


//--------------------------------------------
//routine to instantiate a new kernel thread:
//reserves a place for the thread in the user stack
//initiates a thread object and then fork it for 
//execution
//--------------------------------------------

int do_UserThreadCreate(int f, int arg, int retAdd){
    
    
    //block the main thread from creating  a new thread if there are no space in the userstack
    currentThread->space->maxThreadNumber->P();

    //wrapping the arguments passed to the Thread::Fork()
    wrapper *funcAndArg = new wrapper();
    funcAndArg->func = f;
    funcAndArg->argument=arg ;
    funcAndArg->retAdd = retAdd;  // Add the return address for automatic termination


    //find a place in the userstack bitmap
    currentThread->space->bitmapLock-> P();
    // currentThread->space->bitmapLock->Acquire();
    int stackPos = currentThread->space->myStackMap->Find();
    currentThread->space->bitmapLock-> V();
    // currentThread->space->bitmapLock->Release();
    // DEBUG('3',"ID:: %d \n",ID);

    // currentThread->space->myStackMap->Print();

    if ( stackPos == -1){
        // Debug('3',"maximum number of user threads reached");
        return -1;
    }


    // currentThread->space->threadCountLock-> P();
    //  if (nbUserThread == MAX_THREAD_ALLOWED){
    //      threadCountLock ->V();
    //      DEBUG('3', "maximum number of theads allowed for the user program is reached");
    //      return -1;
    //  }

    currentThread->space->threadCountLock-> P();  
    currentThread->space->nbUserThread++;
    currentThread->space->threadCountLock-> V();

    //block the main thread from halting thte system until all user threads
    //finish execution.
    if (currentThread->space->nbUserThread == 1){
        currentThread->space->threadMainLock-> P();
    }
    
    Thread *newthread = new Thread("new thread");
    if(newthread == NULL){
        DEBUG('3',"error in thread creation\n");
        return -1;
    }


    newthread->SetThreadID(idCounter++);
    newthread->SetStackMapping(stackPos);
    currentThread->space->initializedThread[counter] = newthread;    //FIXME: logic is not persistent,
                                                                    // what if the thread didn't finish but you 
                                                                    //circled the counter and overwritten it.
    counter = (counter+1) % MAX_THREAD_NUMBER_ALLOWED;

    newthread->Fork(StartUserThread,(int) funcAndArg);    

    return newthread->GetThreadID();
}


//--------------------------------------------
//routine to start executing the thread:
//initiates the user stack part of the thread
//by initializing the machine registers and 
//then starts executing the thread routine. 
//--------------------------------------------

void StartUserThread(int f){

    currentThread->space->InitRegisters();
    wrapper *temp = (wrapper*) f;
    
    
    machine->WriteRegister(4,(int) temp->argument);
    machine->WriteRegister(PCReg,(int) temp->func); 
    machine->WriteRegister(NextPCReg,(int)  temp->func+4);

    machine->WriteRegister(RetAddrReg, (int)temp->retAdd);
    machine->WriteRegister(StackReg,(currentThread->space->size) -8 - currentThread->GetStackMapping()* 2*PageSize);
    machine->Run();


}


//--------------------------------------------
//routine to exit a user thread after finishing
//execution, releases it's  userstack reserved
//area and notifies the system to create a 
//thread if there was no place in the stack/
// if it is the last user thread executing,
// release the process main thread if it is waiting 
// to halt the system.
//--------------------------------------------

void do_UserThreadExit()
{

    maxThreadNumber->P();
    totalNumberOfThreads--;
    maxThreadNumber->V();
    
    currentThread->space->threadCountLock-> P();
    currentThread->space->nbUserThread--;
    currentThread->space->threadCountLock-> V();

    //release the user stack area
    currentThread->space->bitmapLock-> P();
    currentThread->space->rmIndex(currentThread->GetStackMapping());
    currentThread->space->bitmapLock-> V();
    // currentThread->space->bitmapLock->Release();
    
    
    //FIXME: pre-emption may cause a problem in the next two steps ? 
    // where the main thread might start working on before the current thread calls finish 
    //notify the process main thread to perform halt if it is waiting 
    if (currentThread->space-> nbUserThread== 0){
        currentThread->space->threadMainLock-> V();
    }

    //notify the system to create new thread if any is waiting
    currentThread->space->maxThreadNumber->V();

    currentThread->Finish();
}

void do_UserThreadJoin(int tid){


    for (int i =0;i<MAX_THREAD_ALLOWED;i++){

        if(currentThread->space->initializedThread[i] != 0 && currentThread->space->initializedThread[i]->GetThreadID() == tid ){
            currentThread->space->initializedThread[i]->ThreadSemaphoreWait();
            break;
        }
    }
}