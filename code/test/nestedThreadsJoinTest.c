
#include "syscall.h"

void routine2(void* c){

    int num = 6;
    PutChar(*(char*) c);
    PutChar('\n');
    PutInt(num);
    PutChar('\n');
    //UserThreadExit();
}

void routine1(char c ){

    char* temp = &c; 
	int tid_2 = UserthreadCreate((void* ) routine2,(void*) temp );
	UserThreadJoin(tid_2);
	PutString("Child thread finished waiting\n");
    //UserThreadExit();

} 



int main()
{
	/* code */
	int tid_1 = UserthreadCreate((void*) routine1, (void*) 'b');
	UserThreadJoin(tid_1);
	PutString("Main thread finished waiting\n");
	Halt();   // Not sure if we should put or not
    /* not reached */
    return 0;
}