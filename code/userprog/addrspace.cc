// addrspace.cc 
//      Routines to manage address spaces (executing user programs).
//
//      In order to run a user program, you must:
//
//      1. link with the -N -T 0 option 
//      2. run coff2noff to convert the object file to Nachos format
//              (Nachos object code format is essentially just a simpler
//              version of the UNIX executable object code format)
//      3. load the NOFF file into the Nachos file system
//              (if you haven't implemented the file system yet, you
//              don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
//#include "FrameProvider.h"

// #include <strings.h>		/* for bzero */

//----------------------------------------------------------------------
// SwapHeader
//      Do little endian to big endian conversion on the bytes in the 
//      object file header, in case the file was generated on a little
//      endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------
static void
SwapHeader (NoffHeader * noffH)
{
    noffH->noffMagic = WordToHost (noffH->noffMagic);
    noffH->code.size = WordToHost (noffH->code.size);
    noffH->code.virtualAddr = WordToHost (noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost (noffH->code.inFileAddr);
    noffH->initData.size = WordToHost (noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost (noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost (noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost (noffH->uninitData.size);
    noffH->uninitData.virtualAddr =
	WordToHost (noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost (noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
//      Create an address space to run a user program.
//      Load the program from a file "executable", and set everything
//      up so that we can start executing user instructions.
//
//      Assumes that the object code file is in NOFF format.
//
//      First, set up the translation from program memory to physical 
//      memory.  For now, this is really simple (1:1), since we are
//      only uniprogramming, and we have a single unsegmented page table
//
//      "executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace (OpenFile * executable)
{
    NoffHeader noffH;
    unsigned int i;
    int frameId;
    nbUserThread=0;
    executable->ReadAt ((char *) &noffH, sizeof (noffH), 0);
    // Thread *initializedThread = new Thread[MAX_THREAD_ALLOWED];

    // ReadAtVirtual(executable, ,sizeof(noffH),0 , pageTable,numPages);

    if ((noffH.noffMagic != NOFFMAGIC) &&
	(WordToHost (noffH.noffMagic) == NOFFMAGIC))
	SwapHeader (&noffH);
    ASSERT (noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize;	// we need to increase the size
    // to leave room for the stack
    // numPages = divRoundUp (size, PageSize) + MAX_BRK_Per_AddrSpace;
    numPages = divRoundUp (size, PageSize);
    size = (numPages * PageSize);
    // brk = (unsigned int)(numPages-divRoundUp(UserStackSize,PageSize) -MAX_BRK_Per_AddrSpace +1);

    // brkMax = brk + MAX_BRK_Per_AddrSpace; 

    // inititalize a frame map here

    if (numPages > (unsigned) machine->frameMap->NumAvailFrame()){
        DEBUG('4',"requested space exceeded number of available pages\n");
        DEBUG('4', " number of available frames %d: number of requested frames: %d",\
        machine->frameMap->NumAvailFrame(),numPages);
        return;
    }
    if(machine->frameMap->NumAvailFrame() == 0){
        DEBUG('4',"no available space for a new process\n");
        return;
    }


    ASSERT (numPages <= NumPhysPages);	// check we're not trying

    DEBUG ('4', "Initializing address space, num pages %d, size %d\n",
	   numPages, size);
// first, set up the translation 
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++)
      {

        //   if (i >=brk && i <=brkMax){
        //       pageTable[i].virtualPage = i;
        //       pageTable[i].valid = FALSE;
        //       pageTable[i].use = FALSE;
        //       pageTable[i].dirty = FALSE;

        //   }

      frameId = machine->frameMap->GetEmptyFrame();
      DEBUG('5',"frame id %d \n",frameId);
	  pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
	  pageTable[i].physicalPage = frameId;
	  pageTable[i].valid = TRUE;
	  pageTable[i].use = FALSE;
	  pageTable[i].dirty = FALSE;
	  pageTable[i].readOnly = FALSE;	// if the code segment was entirely on 
	  // a separate page, we could set its 
	  // pages to be read-only
      }
    
    
// then, copy in the code and data segments into memory
    if (noffH.code.size > 0)
      {
        //   int numFrames = divRoundUp(noffH.code.size,PageSize);
        //   for(i=0;i<numFrames;i++)
        //      freeMap->GetEmptyFrame();

	  DEBUG ('a', "Initializing code segment, at 0x%x, size %d\n",
		 noffH.code.virtualAddr, noffH.code.size);
   
	//   executable->ReadAt (&(machine->mainMemory[noffH.code.virtualAddr]),
	// 		      noffH.code.size, noffH.code.inFileAddr);
      ReadAtVirtual(executable,noffH.code.virtualAddr,
                    noffH.code.size,noffH.code.inFileAddr,
                    pageTable,numPages);
      }


    if (noffH.initData.size > 0)
      {
          //
	  DEBUG ('a', "Initializing data segment, at 0x%x, size %d\n",
		 noffH.initData.virtualAddr, noffH.initData.size);
	//   executable->ReadAt (&
	// 		      (machine->mainMemory
	// 		       [noffH.initData.virtualAddr]),
	// 		      noffH.initData.size, noffH.initData.inFileAddr);
      }

    ReadAtVirtual(executable,noffH.initData.virtualAddr,
                  noffH.initData.size,noffH.initData.inFileAddr,
                  pageTable,numPages);

    myStackMap = new BitMap(MAXUSERTHREAD);
    semaphoreIdMap = new BitMap(MAX_SEMAPHORES_ALLOWED);

    /* declare semaphores for threads control */
    threadCountLock = new Semaphore("Locking count", 1);
    threadMainLock = new Semaphore("main thread lock", 1);
    bitmapLock = new Semaphore("bitmap lock", 1);
    maxThreadNumber = new Semaphore("created threads counter",MAXUSERTHREAD);
    brkLock = new Semaphore("lock on brk pointer",1);
    //lock for semaphore control
    semCreateLock = new Semaphore("sempahore creation lock",1);
    // trial1 = new Semaphore("queue for waiting threads", 1);

    // Object array of type Thread to keep track of threads within the address space
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//      Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace ()
{
  // LB: Missing [] for delete
  // delete pageTable;
  delete [] pageTable;
  delete myStackMap;
  for (unsigned int i = 0; i <  numPages; i++){
      machine->frameMap->ReleaseFrame(pageTable[i].physicalPage);
  }
  delete threadCountLock;
  delete threadMainLock;
  delete bitmapLock;
  delete maxThreadNumber;
  delete semCreateLock;
  delete semaphoreIdMap;
  
//   delete trial1;2`
//   delete [] initializedThread;

  // End of modification
}


/***************************************************************************************/


void 
AddrSpace::ReadAtVirtual(OpenFile * executable, int virtualaddr, int numBytes,
                          int position, TranslationEntry *pageTable, unsigned numPages) {
    
        int readSuccess,i; 
        char *buffer = new char[numBytes];


    	// set the machine page table variable to the current process pageTable. 
        machine->pageTable = pageTable;
        machine->pageTableSize = numPages;


        if(numBytes <=0){
            DEBUG('a',"number of bytes is zero, nothing to write\n");
            return;
        }
        readSuccess = executable->ReadAt(buffer,numBytes,position);

        if(readSuccess == 0){
            DEBUG('a'," number of bytes read from file is zero, failed to read from the file\n");
            return;
        }

        
       	for (i=0; i<numBytes; i++){
       		machine->WriteMem(virtualaddr+i, 1, buffer[i]);
       	}


 }    
// /***************************************************************************************/

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
//      Set the initial values for the user-level register set.
//
//      We write these directly into the "machine" registers, so
//      that we can immediately jump to user code.  Note that these
//      will be saved/restored into the currentThread->userRegisters
//      when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters ()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister (i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister (PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister (NextPCReg, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we don't
    // accidentally reference off the end!
    machine->WriteRegister (StackReg, numPages * PageSize - 16);
    DEBUG ('a', "Initializing stack register to %d\n",
	   numPages * PageSize - 16);
}



//----------------------------------------------------------------------
// AddrSpace::SaveState
//      On a context switch, save any machine state, specific
//      to this address space, that needs saving.
//
//      For now, nothing!
//----------------------------------------------------------------------

void
AddrSpace::SaveState ()
{
    pageTable = machine->pageTable;
    numPages = machine->pageTableSize;
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
//      On a context switch, restore the machine state so that
//      this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void
AddrSpace::RestoreState ()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}


int AddrSpace::GetSize(){
    return size;
}

void
AddrSpace::rmIndex(int index){

	myStackMap->Clear(index);
}

void AddrSpace::setPid(int id){
    pid = id;
}

int  AddrSpace::getPid(){
    return pid;
}


int AddrSpace::Sbrk(unsigned int n){


    brkLock->P();
    if(brk + n > brkMax || (unsigned int)machine->frameMap->NumAvailFrame() < n){
        DEBUG('4',"no availabe frames to extend the brk\n");
        brkLock->V();
        return  -1;
    }

    for(unsigned int i =0;i<n;i++){
        // int frameId = machine->frameMap->AllocateEmptyPge(); 
        int frameId = machine->frameMap->GetEmptyFrame();
        pageTable[brk+i].physicalPage= frameId;
        pageTable[brk+i].valid = TRUE;
    }

    unsigned int oldBrk = brk;
    brk = brk+n;
    brkLock->V();
    return  oldBrk *(unsigned int) PageSize;



}
