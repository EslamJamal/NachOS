
#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H
#include "copyright.h"
#include "bitmap.h"
//#include "synch.h"
// class Semaphore;
class FrameProvider{
	public:

		FrameProvider(int numPages);
		~FrameProvider();
		// Semaphore *frameMapLock;
		int GetEmptyFrame();
		void ReleaseFrame(int frame_num);
		int NumAvailFrame();
		int AllocateEmptyPage();
		void FreePage(int frameId);

	private:
		
		BitMap *frameMap;
		// Semaphore *memoryLock;

};

#endif
