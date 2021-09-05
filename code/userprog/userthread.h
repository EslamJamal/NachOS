
//used CHANGED directive just for consistency among the codebase.
// #ifdef CHANGED
// #ifndef USERTHREAD_H
// #define USERTHREAD_H

#include "utility.h"
#include "machine.h"
#include "addrspace.h"
#include "bitmap.h"


//int nbUserThread = 0;
static int idCounter=0;
static int counter=0;
extern BitMap *myStackMap;
extern Semaphore *maxThreadNumber;

extern int do_UserThreadCreate(int f, int arg, int retAdd);  
extern void StartUserThread(int f);
extern void do_UserThreadExit();

typedef struct {
    int func;
    int argument;
    int retAdd;
} wrapper;



// #endif
// #endif