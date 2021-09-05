

#include "syscall.h"
#define THIS "aaa"
#define THAT "bbb"
const int N = 100; // Choose it large enough!

void puts(char *s){
        char *p;
        for (p = s; *p != '\0'; p++) 
            PutChar(*p);
    }

void f(void *s){
        int i; 
        for (i = 0; i < N; i++) 
            puts((char *)s);
        PutChar('\n');
    }

int main()
{
    PutString("starting forked process routine\n");
    UserthreadCreate((void*) f, (void *) THAT);
    // Halt()
    // PutString("ending forked process routine\n");

    ProcessTerminate();
    /* not reached */
    return 0;
}