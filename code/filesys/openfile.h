// openfile.h 
//	Data structures for opening, closing, reading and writing to 
//	individual files.  The operations supported are similar to
//	the UNIX ones -- type 'man open' to the UNIX prompt.
//
//	There are two implementations.  One is a "STUB" that directly
//	turns the file operations into the underlying UNIX operations.
//	(cf. comment in filesys.h).
//
//	The other is the "real" implementation, that turns these
//	operations into read and write disk sector requests. 
//	In this baseline implementation of the file system, we don't 
//	worry about concurrent accesses to the file system
//	by different threads -- this is part of the assignment.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef OPENFILE_H
#define OPENFILE_H

#include "copyright.h"
#include "utility.h"

#ifdef FILESYS_STUB			// Temporarily implement calls to 
					// Nachos file system as calls to UNIX!
					// See definitions listed under #else
class OpenFile {
  public:
    OpenFile(int f) { file = f; currentOffset = 0; }	// open the file
    ~OpenFile() { Close(file); }			// close the file

    int ReadAt(char *into, int numBytes, int position) { 
    		Lseek(file, position, 0); 
		return ReadPartial(file, into, numBytes); 
		}	
    int WriteAt(const char *from, int numBytes, int position) { 
    		Lseek(file, position, 0); 
		WriteFile(file, from, numBytes); 
		return numBytes;
		}	
    int Read(char *into, int numBytes) {
		int numRead = ReadAt(into, numBytes, currentOffset); 
		currentOffset += numRead;
		return numRead;
    		}
    int Write(const char *from, int numBytes) {
		int numWritten = WriteAt(from, numBytes, currentOffset); 
		currentOffset += numWritten;
		return numWritten;
		}

    int Length() { Lseek(file, 0, 2); return Tell(file); }
    
  private:
    int file;
    int currentOffset;
};

#else // FILESYS
class FileHeader;
typedef struct seekChainedList {
    int seekPosition;
    int threadId;
    struct seekChainedList *next;
} seekList_t;
class OpenFile {
  public:
    OpenFile(int sector);		// Open a file whose header is located
					// at "sector" on the disk
			//done
    ~OpenFile();			// Close the file
			//done
    void Seek(int position, int threadId = 0); 		// Set the position from which to 
					// start reading/writing -- UNIX lseek
			//done
    int Read(char *into, int numBytes, int threadId = 0); // Read/write bytes from the file,
					// starting at the implicit position.
					// Return the # actually read/written,
					// and increment position in file.
			//done
    int Write(const char *from, int numBytes, int ThreadId = 0);
			//done
    int ReadAt(char *into, int numBytes, int position);
    					// Read/write bytes from the file,
					// bypassing the implicit position.
			//done
    int WriteAt(const char *from, int numBytes, int position);
			//done
    int Length(); 			// Return the number of bytes in the
					// file (this interface is simpler 
					// than the UNIX idiom -- lseek to 
					// end of file, tell, lseek back
			//done
	int sector_get();
			//done
    void seekPosition_add(int threadId);
			//done
    bool seekPosition_del(int threadId);
			//done
    bool alreadyOpened();
			//done
	bool isDir();
			//done
  private:
    FileHeader *hdr;			// Header for this file
	seekList_t *seekList;		// List of tuples : current position within the file ; threadId
	int seekPosition_get(int threadId);
		//done
	void seekPosition_set(int threadId, int seekPosition);
		//done
};

#endif // FILESYS

#endif // OPENFILE_H
