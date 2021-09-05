// exception.cc 
//      Entry point into the Nachos kernel from user programs.
//      There are two kinds of things that can cause control to
//      transfer back to here from user code:
//
//      syscall -- The user code explicitly requests to call a procedure
//      in the Nachos kernel.  Right now, the only function we support is
//      "Halt".
//
//      exceptions -- The user code does something that the CPU can't handle.
//      For instance, accessing memory that doesn't exist, arithmetic errors,
//      etc.  
//
//      Interrupts (which can also cause control to transfer from user
//      code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synch.h"

extern int do_UserThreadCreate(int f, int arg, int retAdd);  
extern void do_UserThreadExit();
extern void do_UserThreadJoin(int tid);

extern int do_ForkAndExec(char *filename);
extern void do_exitProcess();


#ifndef SEMT_H
#define SEMT_H
typedef int sem_t;
#endif

extern sem_t do_UserSemaphoreCreate(char *name,int value);
extern bool do_UserSemaphoreDestroy(sem_t UserSemaphore);
extern void do_UserSemaphoreProberen(sem_t UserSemaphore);
extern void do_UserSemaphoreVerhogen(sem_t UserSemaphore);

#ifndef SEMT_H
#define SEMT_H
typedef int sem_t;
#endif

extern sem_t do_UserSemaphoreCreate(char *name,int value);
extern bool do_UserSemaphoreDestroy(sem_t UserSemaphore);
extern void do_UserSemaphoreProberen(sem_t UserSemaphore);
extern void do_UserSemaphoreVerhogen(sem_t UserSemaphore);


extern Semaphore *threadMainLock;
extern Semaphore *maxThreadNumber;

extern Semaphore *threadMainLock;
extern Semaphore *maxThreadNumber;

//----------------------------------------------------------------------
// UpdatePC : Increments the Program Counter register in order to resume
// the user program immediately after the "syscall" instruction.
// ----------------------------------------------------------------------
static void
UpdatePC ()
{
    int pc = machine->ReadRegister (PCReg);
    machine->WriteRegister (PrevPCReg, pc);
    pc = machine->ReadRegister (NextPCReg);
    machine->WriteRegister (PCReg, pc);
    pc += 4;
    machine->WriteRegister (NextPCReg, pc);
}

void copyStringFromMachine(int from, char *to, unsigned size){
  // char* mips;

  // if (size > MAX_STRING_SIZE){
  //   // We cut the string if it's too long
  //   size = MAX_STRING_SIZE;
  // }

  // for (unsigned i = 0; i < size; i++){
  //   mips = &machine->mainMemory[from + i];
  //   *(to + i) = *mips;
  // }
  // *(to + size) = '\0'; // do not forget end of file.

  int value;
  int i=0;
  // char *c[MAX_STRING_SIZE];
do{

    machine->ReadMem(from+i,1,&value);
    *(to+i) = (char) value;
    DEBUG('f',"char is: %c\n",(char) value);
    i++;

}while(i<MAX_STRING_SIZE-1 && *(to+i-1) != '\0');

for (int j=i;j<MAX_STRING_SIZE-1;j++)
    *(to+i) = '\0';


  // for(i=0;i<MAX_STRING_SIZE;i++){
  //   if (*(to + i-1) == '\0')
      // break; 
    // *(to + i) = (char) value;
  // }

// for (int j=i;j<MAX_STRING_SIZE;j++)
//     *(to + j) = '\0';

}
//----------------------------------------------------------------------
// ExceptionHandler
//      Entry point into the Nachos kernel.  Called when a user program
//      is executing, and either does a syscall, or generates an addressing
//      or arithmetic exception.
//
//      For system calls, the following is the calling convention:
//
//      system call code -- r2
//              arg1 -- r4
//              arg2 -- r5
//              arg3 -- r6
//              arg4 -- r7
//
//      The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//      "which" is the kind of exception.  The list of possible exceptions 
//      are in machine.h.
//----------------------------------------------------------------------


void
ExceptionHandler(ExceptionType which)
{
  int type = machine->ReadRegister(2);


  
  if (which == SyscallException) {
    switch (type) {
      case SC_Halt:{
        DEBUG('3',"waiting on the halt semaphore\n");
        currentThread->space->threadMainLock-> P();
        DEBUG('a', "Shutdown, initiated by user program.\n");
        interrupt->Halt();
        break;
        }

      case SC_PutChar: {
        char reg4 = (char)machine->ReadRegister(4);
        synch_console->SynchPutChar(reg4);
        break;
        }

      case SC_GetChar: {
        char ch = synch_console->SynchGetChar();
        machine->WriteRegister(2, (int)ch);
        //printf("the character is here %c\n", ch);
        break;
      }
  	  case SC_USTHREADCREATE: {

  		  // handles the syscall to create a new user thread.

        maxThreadNumber->P();
        totalNumberOfThreads ++;
        DEBUG('3',"total number of threads requested : %d \n",totalNumberOfThreads);
        maxThreadNumber->V();

        if(totalNumberOfThreads == MAX_THREAD_NUMBER_ALLOWED){
            DEBUG('3',"maximum number of threads for the system is reached\n");
            // machine->WriteRegister(2, -1);
            ASSERT(FALSE);
            break;
        }

    		int f = machine->ReadRegister(4);
    		int arg = machine->ReadRegister(5);
        int retAdd = machine->ReadRegister(6);
        int threadID = (int) do_UserThreadCreate(f, arg, retAdd);
        machine->WriteRegister(2, threadID);
    		break;
  	  }

      case SC_USTHREADJOIN:{
         //handles the syscall to join a user thread

        int tid = machine->ReadRegister(4);
        do_UserThreadJoin(tid); 
        break;

      }

      case SC_USTHREADEXIT: {
        do_UserThreadExit();
        break;
      }

      case SC_FORKEXEC:{
          int reg4 = machine->ReadRegister(4);
        // int reg5 = machine->ReadRegister(5);
        // DEBUG('5',"value of register 5: %d\n",reg5);
        char* s = (char*)&machine->mainMemory[reg4];
        do_ForkAndExec(s);
        break;
      }


      case SC_PROCESSTERMINATE:{
        do_exitProcess();        // trial1->V();

        break;
      }

      case SC_USERSEMAPHORECREATE:{
        
        int reg4 = machine->ReadRegister(4);
        int reg5 = machine->ReadRegister(5);

        char* name = (char*)&machine->mainMemory[reg4];

        sem_t tempSemId = do_UserSemaphoreCreate(name, reg5);
        machine->WriteRegister(2,(int)tempSemId);

        break;

      }

      case SC_USERSEMAPHOREDESTROY:{
        int reg4 = machine->ReadRegister(4);
        sem_t arg = ((sem_t) reg4);
        bool status = do_UserSemaphoreDestroy(arg);
        //todo: return back the status 
        machine->WriteRegister(2,(int)status);

        break;
      }

      case SC_USERSEMAPHOREPROBEREN:{
        int reg4 = machine->ReadRegister(4);
        sem_t arg = ((sem_t) reg4);
        do_UserSemaphoreProberen(arg);
        break;
      }


      case SC_USERSEMAPHOREVERHOGEN:{
        int reg4 = machine->ReadRegister(4);
        sem_t arg = ((sem_t) reg4);
        do_UserSemaphoreVerhogen(arg);
        break;
      }

      case SC_SBRK:{

        int requestedSize = machine->ReadRegister(4);
        int ptr = currentThread->space->Sbrk(requestedSize);
        machine->WriteRegister(2,ptr);
        break;
      }

        case SC_PutString: {
          DEBUG('s', " handling PutString syscall.\n");
          char s[MAX_STRING_SIZE];
          int startofString = machine->ReadRegister(4);
          copyStringFromMachine(startofString, s, MAX_STRING_SIZE);
          DEBUG('s',"string is: %s\n",s);
          synch_console->SynchPutString(s);
          break;
        }

        case SC_GetString: {
          DEBUG('s', " handling GetString syscall.\n");
          int reg4 = machine->ReadRegister(4);
          int reg5 = machine->ReadRegister(5);
          synch_console->SynchGetString((char*)&machine->mainMemory[reg4], reg5);
          break;
        }

        case SC_PutInt: {
          DEBUG('s', " handling PutInt syscall.\n");
          int reg4 = machine->ReadRegister(4);
          synch_console->SynchPutInt(reg4);
          break;
        }

        case SC_GetInt: {
          DEBUG('s', " handling GetInt syscall.\n");
          int reg4 = machine->ReadRegister(4);
          synch_console->SynchGetInt((int*)&machine->mainMemory[reg4]);
          break;
        }

        default: {
          printf("Unexpected user mode exception %d %d\n", which, type);
          ASSERT(FALSE);
          }
      }
  UpdatePC();
  }
}
