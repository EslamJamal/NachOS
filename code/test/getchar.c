#include "syscall.h"

char scan()
{
	return(GetChar());
}

void print(char c)
{
	PutChar(c);
	PutChar('\n');
}


int
main()
{
	char ch = scan();
	print(ch);
	Halt();
	/* not reached */
    return 0;
}