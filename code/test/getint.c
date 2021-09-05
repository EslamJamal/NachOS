#include "syscall.h"

int main(){
    int number;
    GetInt(&number);
    PutInt(number);
    PutChar('\n');
    Halt();
    /* not reached */
    return 0;
}