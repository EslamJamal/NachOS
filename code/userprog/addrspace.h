// addrspace.h 
//      Data structures to keep track of executing user programs 
//      (address spaces).
//
//      For now, we don't keep any information about address spaces.
//      The user level CPU state is saved and restored in the thread
//      executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "bitmap.h"

#define UserStackSize		1024	// increase this as necessary!
#define MAXUSERTHREAD UserStackSize / (2*PageSize)
#define MAX_THREAD_ALLOWED 20
#define MAX_SEMAPHORES_ALLOWED 10

#define MAX_BRK_Per_AddrSpace 100

class Semaphore;
class Thread;
class AddrSpace
{
  public:
    unsigned int size;
    BitMap *myStackMap;
    Semaphore *threadMainLock;
    Semaphore *threadCountLock;
    Semaphore *bitmapLock;
    Semaphore *maxThreadNumber; //blocks thread creation when reaching the limit,
                                // and allows it when a free space in the stack is available. 
    // Semaphore *trial1;
    int pid;
    int nbUserThread;
    Thread *initializedThread[MAX_THREAD_ALLOWED]; //FIXME: what is max thread allowed 

    Semaphore *semCreateLock;
    Semaphore *userLevelSemaphores[MAX_SEMAPHORES_ALLOWED];
    Semaphore *brkLock;
    BitMap *semaphoreIdMap;


    AddrSpace (OpenFile * executable);	// Create an address space,
    // initializing it with the program
    // stored in the file "executable"
    ~AddrSpace ();		// De-allocate an address space

    void InitRegisters ();	// Initialize user-level CPU registers,
    // before jumping to user code

    void SaveState ();		// Save/restore address space-specific
    void RestoreState ();	// info on a context switch 
    int GetSize();
    void rmIndex(int index);
    void setPid(int id);
    int getPid();
    int Sbrk(unsigned int n);
    static void ReadAtVirtual(OpenFile * executable, int virtualaddr, int numBytes,
                          int position, TranslationEntry *pageTable, unsigned numPages);

  private:
      TranslationEntry * pageTable;	// Assume linear page table translation
    // for now!

    unsigned int numPages;	// Number of pages in the virtual 
    unsigned  int brk;
    unsigned int brkMax;
    // address space

};

#endif // ADDRSPACE_H
