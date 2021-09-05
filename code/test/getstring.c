#include "syscall.h"

int main(){
    PutString("what's your name ?");
    char str[8];
    GetString(str, 8);
    PutString("Hi Mr ");
    PutString(str);
    Halt();

    /* not reached */
    return 0;
}