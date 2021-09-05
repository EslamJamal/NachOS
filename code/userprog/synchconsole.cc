
#include "copyright.h"
#include "system.h"
#include "synchconsole.h"
#include "synch.h"

static Semaphore *readAvail;
static Semaphore *writeDone;
static Semaphore *writeLock;
static Semaphore *readLock;

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

SynchConsole::SynchConsole(char *readFile, char *writeFile)
{
readAvail = new Semaphore("read avail", 0);
writeDone = new Semaphore("write done", 0);
writeLock = new Semaphore("write lock on console",1);
readLock = new Semaphore("read lock on console",1);
console = new Console(readFile, writeFile, ReadAvail, WriteDone, 0);
}

/* deconstructor. DONT TOUCH */
SynchConsole::~SynchConsole()
{
delete console;
delete writeDone;
delete readAvail;
delete writeLock;

}

void SynchConsole::SynchPutChar(const char ch)
{
	// Put a char to the shell
	writeLock->P();
	console->PutChar(ch);
	// DEBUG('3',"putchar done\n");
	writeDone-> P();
	// DEBUG('3',"write is done\n");
	writeLock->V();

} 

char SynchConsole::SynchGetChar()
{
	readAvail->P();
	readLock->P();
	// DEBUG('3',"curent thread in getChar %d\n",(int) currentThread);
	// DEBUG('3',"please input a character \n");
	char ch = console->GetChar();
	/* return the Char at the end */ 
	readLock->V();
	// DEBUG('3',"getChar done\n");
	return(ch);
}

void SynchConsole::SynchPutString(const char s[])
{
	writeLock->P();
	int i = 0;
	while (s[i] != '\0'){ // while not reaching end
		// SynchPutChar(s[i]);
		// DEBUG('3',"char is %c\n",s[i]);
		console->PutChar(s[i]);
		writeDone-> P();
		i++;
	}
	writeLock->V();
}

void SynchConsole::SynchGetString(char *s, int n)
{
	for(int i = 0; i < n; i++){
		s[i] = SynchGetChar();
	}
	s[n + 1] = EOF;
}

void SynchConsole::SynchPutInt(int n){
  char number[8];
  snprintf(number,8,"%d",n); // int snprintf(char *str, size_t size, const char *format, ...);
  SynchPutString(number);
}


void SynchConsole::SynchGetInt(int *n){
	// Only positive integers
	char current;
	char toscan[8];
	int i = 0;
	current = SynchGetChar();
	while ((current >= '0') && (current <= '9') && i < 8){
		toscan[i] = current;
		i++;
		current = SynchGetChar();
	}
  toscan[i] = '\0';
  sscanf(toscan,"%d",n);
}