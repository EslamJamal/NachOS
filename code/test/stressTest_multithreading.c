#include "syscall.h"
#define threads 1000

void routine(char args){
	//PutString("Hello World!");
	PutChar((char)args);
	UserThreadExit();
}

int *createManyThreads(){
	static int pool[threads];  // Must be static to preserve it in the stack after returning. 
	int i;

	for(i=0; i<threads; i++)
		pool[i] = UserthreadCreate((void*) routine,(void*) 'b'); 
	
	return pool;
}



int main(){

	int *ptr;
	ptr = createManyThreads();
	int i;
	
	for(i=0; i<threads; i++)
		PutInt(*(ptr+1));
	
	Halt();
	return 0;
}
