#include "system.h"
#include "FrameProvider.h"
#include <strings.h>		/* for bzero */

/* include the appropriate header file to avoid any dependencies */

FrameProvider::FrameProvider(int numFrames){

	/* initialize a bitmap object */ 
	frameMap = new BitMap(numFrames);

	/* Semaphore for the number */
	// frameMapLock = new Semaphore("Lock for the frame provider", 1); 

}

FrameProvider::~FrameProvider(){

	/* delete the bitmap object */
	delete frameMap;
	// delete frameMapLock;

}

int FrameProvider::GetEmptyFrame(){
	/* we have to first know how to get an empty frame,
	 maybe randomly or first empty free frame then return */
	//  frameMapLock-> P();
	 int frame_num = frameMap->Find();
	// int frame_num = frameMap->FindRandom();
	
	//TODO: check before the bzero if there are no frames available

	// TODO: put bzero here
	/* - In the AddrSpace() we use this bzero (machine->mainMemory, size); such that size is 
	   	numPages * PageSize. So here we want to write bzero(frame object itself returned, PageSize).

	   - Figure out how to fetch the address of the empty frame. 

	   - 21/01/2019 ==> I guess the start of each page should be the page size multiplied by the number
	    itself since the page sizes are the same size 128 */
	bzero(&machine->mainMemory[frame_num * PageSize], PageSize);
	// frameMapLock-> V();
	return frame_num;  // By using Find(), we choose to return first empty


}
// we do not need a lock here
void FrameProvider::ReleaseFrame(int frame_num){
	/* check firt if this frame_num exists */

	/* use Clear() from bitmap to clear the object */ 
	frameMap->Clear(frame_num);

}

int FrameProvider::NumAvailFrame(){
	/* return number of available frames */ 
	// frameMapLock-> P();
	int frame_num = frameMap->NumClear();
	// frameMapLock-> V();

	return frame_num;

}

int FrameProvider::AllocateEmptyPage(){

	//TODO: lock this operation
	int frameId = GetEmptyFrame();

	if(frameId ==-1){
		DEBUG('4',"failed to allocate an empty page\
		no available space in the memory\n");
		return -1;
	}
	

	return frameId;


}


void FrameProvider::FreePage(int frameId){

	ReleaseFrame(frameId);



}