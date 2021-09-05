

//-------------------------------------------------
// File: userSynch.cc 
// Use:  system call handles for user level semaphores.
// @author: Mohammed Almarakby.
//-------------------------------------------------

#include "system.h"
#include "userSynch.h"
#include "thread.h"

// #include "userthread.h"



sem_t do_UserSemaphoreCreate(char *name,int value){

    currentThread->space->semCreateLock->P();
    int semaphoreId = currentThread->space->semaphoreIdMap->Find();

    if(semaphoreId == -1){
        DEBUG('3',"maximum number of semaphores per address space limit exceeded\n");
        currentThread->space->semCreateLock->V();
        return (sem_t) -1;
    }

    currentThread->space->userLevelSemaphores[semaphoreId] = new Semaphore(name,value);
    currentThread->space->semCreateLock->V();

    return (sem_t) semaphoreId;
}

bool do_UserSemaphoreDestroy(sem_t UserSemaphore){

    //todo: updates to be a producer consumer problem
    // is it or not, sine we arenot consuming elements rather than accessing by index, we just need a lock ? 

    bool  semaphoreAvail = currentThread->space->semaphoreIdMap->Test(UserSemaphore);

    if(semaphoreAvail == false){
        DEBUG('3',"semaphore not initialized to be destroyed\n");
        return FAILED;
    }

    currentThread->space->semCreateLock->P();
    delete currentThread->space->userLevelSemaphores[UserSemaphore];
    currentThread->space->userLevelSemaphores[UserSemaphore] = NULL;
    currentThread->space->semCreateLock->V();

    currentThread->space->semaphoreIdMap->Clear(UserSemaphore);
    return SUCCESS;     
}

void do_UserSemaphoreProberen(sem_t UserSemaphore)
{
    
    int semId = (int) UserSemaphore;
    if(currentThread->space->semaphoreIdMap->Test(semId) == false){
        DEBUG('3',"semaphore not initialized to be waited\n");
        return ; //FIXME: should i raise exceptios?
    }

    // Semaphore *temp = currentThread->space->userLevelSemaphores[semId];
    // temp->P();
    // delete temp;
    currentThread->space->userLevelSemaphores[semId] ->P();
}

void do_UserSemaphoreVerhogen(sem_t UserSemaphore){

        int semId = (int) UserSemaphore;

    if(currentThread->space->semaphoreIdMap->Test(semId) == false){
        DEBUG('3',"semaphore not initialized to be posted\n");
        return ; //FIXME: should i raise exceptios?
    }

    // Semaphore *temp = currentThread->space->userLevelSemaphores[semId];
    // temp->V();
    // delete temp;
    currentThread->space->userLevelSemaphores[semId] ->V();


}

