

#include "system.h"
#include "userprocess.h"
#include "thread.h"


int do_ForkAndExec(char *filename){

	//TODO: check how many process we can provide
	
	// ADD: Sanity check if no process available to create 
	Thread *ProcessMainThread = new Thread("processmainthread");
	int oldPid;

	wrapper *args = new wrapper();
	args->filename = filename;
	args-> pid = pid;

	ProcessMainThread-> Fork(StartNewProcess, (int)  args);

	numProcess-> P();
	oldPid = pid;
	pid++;
	numUserProcess++;
	numProcess-> V();
	return oldPid;
}



void StartNewProcess(int args){

	wrapper *temp = (wrapper*) args;
	OpenFile *executable = fileSystem->Open(temp->filename);


	AddrSpace *ProcessSpace;
	//TODO: update addresspspaceclass to a pid field

	if(executable == NULL){
		DEBUG('4',"unable to open the file: %s\n",temp->filename);
		return;
	}

	ProcessSpace = new AddrSpace(executable);
	delete executable;


	ProcessSpace->setPid(temp->pid);
	currentThread->space = ProcessSpace;

	ProcessSpace->InitRegisters();
	// ProcessSpace->RestoreState();

	machine->Run();

}

void do_exitProcess(){
	// ADD: Sanity check if no process is running. Either by checking on a a PID or if process exsists
	/*if current thread exists*/
	IntStatus oldLevel = interrupt->SetLevel(IntOff); // close interrupt to prevent switching 

	DEBUG('4',"inside exit process\n");
	if(currentThread->space->nbUserThread != 0){
        DEBUG('4',"waiting for the the user threads to finish\n");
		currentThread->space->threadMainLock->P();
			DEBUG('4',"User threads finished\n");
	}

	numProcess-> P();
	numUserProcess--; 
	numProcess-> V();

	//TODO: delete address space using the destructor to releasse the mainMemory frames 

	// AddrSpace *temp = currentThread->space;
	// delete temp;
	// currentThread->space = NULL;
	//Thread *destroyThread = currentThread;

	delete currentThread->space;

	/* if num of proesses is 0 do interrupt halt otherwise finish the current thread*/
	if(numUserProcess == 0){
		interrupt->Halt(); // exit because it is the last process
	}else{
		currentThread->Finish();
	}

	interrupt->SetLevel(oldLevel);
}