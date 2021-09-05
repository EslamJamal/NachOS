

#include "syscall.h"






int main(){

    int ptr = Sbrk(51*128);
    if (ptr == -1){
        PutString("requested size is mpre than the availabe memory for a process\n");
        PutString("closing the process...\n");
        Halt();
    }

    PutString("ptr to the new requested memory block\n");
    PutInt(ptr);   
    PutChar('\n');

    Halt();
    /* not reached */
    return 0;

}