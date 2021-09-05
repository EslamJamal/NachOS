// synch.cc 
//      Routines for synchronizing threads.  Three kinds of
//      synchronization routines are defined here: semaphores, locks 
//      and condition variables (the implementation of the last two
//      are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
//      Initialize a semaphore, so that it can be used for synchronization.
//
//      "debugName" is an arbitrary name, useful for debugging.
//      "initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore (const char *debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
//      De-allocate semaphore, when no longer needed.  Assume no one
//      is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore ()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
//      Wait until semaphore value > 0, then decrement.  Checking the
//      value and decrementing must be done atomically, so we
//      need to disable interrupts before checking the value.
//
//      Note that Thread::Sleep assumes that interrupts are disabled
//      when it is called.
//----------------------------------------------------------------------

void
Semaphore::P ()
{
    IntStatus oldLevel = interrupt->SetLevel (IntOff);	// disable interrupts

    while (value == 0)
      {				// semaphore not available
	  queue->Append ((void *) currentThread);	// so go to sleep
	  currentThread->Sleep ();
      }
    value--;			// semaphore available, 
    // consume its value

    (void) interrupt->SetLevel (oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
//      Increment semaphore value, waking up a waiter if necessary.
//      As with P(), this operation must be atomic, so we need to disable
//      interrupts.  Scheduler::ReadyToRun() assumes that threads
//      are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V ()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel (IntOff);

    thread = (Thread *) queue->Remove ();
    if (thread != NULL)		// make thread ready, consuming the V immediately
	scheduler->ReadyToRun (thread);
    value++;
    (void) interrupt->SetLevel (oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock (const char *debugName)
{
    /* For a lock creation we need a queue for waiting threads */ 
    waitingQueue = new List;
    name = debugName;
    grap = FALSE;
}

Lock::~Lock ()
{
    delete waitingQueue;
}
void
Lock::Acquire ()
{
    /* first we need to know where t put the disable then append
    1 to the queue as long as IsEmpty returns True */

    IntStatus oldLevel = interrupt->SetLevel (IntOff);
    /* if True means there is a thread with the lock */
    if (grap){
        waitingQueue->Append((void *)currentThread);
        currentThread->Sleep();
    }else
        grap = TRUE;
    

    (void) interrupt->SetLevel (oldLevel);
}
void
Lock::Release ()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel (IntOff);

    thread = (Thread *) waitingQueue->Remove ();
    if (thread != NULL)     
        scheduler->ReadyToRun (thread);

    grap = FALSE;
    (void) interrupt->SetLevel (oldLevel);
}

Condition::Condition (const char *debugName)
{
    /* We need a queue for waiting threads */ 
    name =debugName;
    waitingQueue = new List;
    //cond_QueueLock = new Lock("Lock for the condition variable");
}

Condition::~Condition ()
{
    delete waitingQueue;
    //delete cond_QueueLock;
}
void
Condition::Wait (Lock * conditionLock)
{
    /* Here, wait assumes that the lock is already held so all it has to do 
    is to release it for another thread then puts the caller on sleep */
    //ASSERT (FALSE);

    //cond_QueueLock->Acquire ();
    // Release the mutex passed to the wait
   //  conditionLock->Release();
   //  // Append it on the queue
   //  currentThread->Sleep();
   //  waitingQueue->Append((void *) currentThread);
   //  // Put the running thread on sleep
   // // cond_QueueLock->Release ();

   //  conditionLock-> Acquire();

    IntStatus oldLevel = interrupt->SetLevel (IntOff);

    if (conditionLock->grap){
        waitingQueue->Append((void *)currentThread);
        conditionLock->Release();
        currentThread->Sleep();
    }else{
        conditionLock->Release();
        currentThread->Sleep();
    }
    //conditionLock->Acquire();
    (void) interrupt->SetLevel (oldLevel);
}

void
Condition::Signal (Lock * conditionLock)
{
    /* I think signal will never be called unless the
       the lock is held so no need for it here I guess */
    //cond_QueueLock->Acquire ();
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel (IntOff);
    thread = (Thread *) waitingQueue->Remove ();
    if (thread != NULL)     
        scheduler->ReadyToRun (thread);
    //cond_QueueLock->Release ();
    (void) interrupt->SetLevel (oldLevel);

}
void
Condition::Broadcast (Lock * conditionLock)
{
    /* same logic by removing from the list all the threds */ 

    //cond_QueueLock->Acquire ();
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel (IntOff);

    while(!waitingQueue->IsEmpty()){
        thread = (Thread *) waitingQueue->Remove ();
        if (thread != NULL)     
            scheduler->ReadyToRun (thread);
        else
            continue;
    }
    //cond_QueueLock->Release ();
    (void) interrupt->SetLevel (oldLevel);

}
