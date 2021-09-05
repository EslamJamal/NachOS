// filesys.h 
//	Data structures to represent the Nachos file system.
//
//	A file system is a set of files stored on disk, organized
//	into directories.  Operations on the file system have to
//	do with "naming" -- creating, opening, and deleting files,
//	given a textual file name.  Operations on an individual
//	"open" file (read, write, close) are to be found in the OpenFile
//	class (openfile.h).
//
//	We define two separate implementations of the file system. 
//	The "STUB" version just re-defines the Nachos file system 
//	operations as operations on the native UNIX file system on the machine
//	running the Nachos simulation.  This is provided in case the
//	multiprogramming and virtual memory assignments (which make use
//	of the file system) are done before the file system assignment.
//
//	The other version is a "real" file system, built on top of 
//	a disk simulator.  The disk is simulated using the native UNIX 
//	file system (in a file named "DISK"). 
//
//	In the "real" implementation, there are two key data structures used 
//	in the file system.  There is a single "root" directory, listing
//	all of the files in the file system; unlike UNIX, the baseline
//	system does not provide a hierarchical directory structure.  
//	In addition, there is a bitmap for allocating
//	disk sectors.  Both the root directory and the bitmap are themselves
//	stored as files in the Nachos file system -- this causes an interesting
//	bootstrap problem when the simulated disk is initialized. 
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef FS_H
#define FS_H

#include "copyright.h"
#include "openfile.h"
#include "filehdr.h"

#define MAX_OPEN_FILE 10
#define ROOT_DIR 0
#define CURR_DIR 1

#ifdef FILESYS_STUB 		// Temporarily implement file system calls as 
				// calls to UNIX, until the real file system
				// implementation is available
class FileSystem {
  public:
    FileSystem(bool format) {}

    bool Create(const char *name, int initialSize) { 
	int fileDescriptor = OpenForWrite(name);

	if (fileDescriptor == -1) return FALSE;
	Close(fileDescriptor); 
	return TRUE; 
	}

    OpenFile* Open(char *name) {
	  int fileDescriptor = OpenForReadWrite(name, FALSE);

	  if (fileDescriptor == -1) return NULL;
	  return new OpenFile(fileDescriptor);
      }

    bool Remove(char *name) { return Unlink(name) == 0; }

};

#else // FILESYS

typedef struct table {
    int threadId;
    OpenFile** OpenTable;
    struct table *next;
} table_t;

typedef struct system_table {
    OpenFile *openFile;
    struct system_table *next;
} system_table_t;

class FileSystem {
  public:
    FileSystem(bool format);		// Initialize the file system.
					// Must be called *after* "synchDisk" 
					// has been initialized.
    					// If "format", there is nothing on
					// the disk, so initialize the directory
    					// and the bitmap of free blocks.
					//done
	~FileSystem();	
					//done
    bool Create(const char *name, int initialSize);  	
					// Create a file (UNIX create)
					//done
    OpenFile* Open(const char *name, int threadId = 0); 	// Open a file (UNIX open)
					//done
    bool Remove(const char *name, int threadId = 0); 	// Delete a file (UNIX unlink)
					//done
	bool ChangeDirectory(const char *name, int threadId = 0);
					//done
	bool MakeDirectory(const char *name, int threadId = 0);
					//done
	bool RemoveDirectory(const char *name, int threadId = 0);
					//done
	void GetInTable(int* table, int threadId);
					//done
	bool GetOutTable(int threadId);
					//done
    void List();			// List all the files in the file system
					//done
    void Print();			// List all the files and their contents
					//done
  private:
  	system_table_t *SystemTable;
	table_t *PerThreadTable;
  	OpenFile* freeMapFile;		// Bit map of free disk blocks,
					// represented as a file

	void createPerThreadTable();
					//done
	void createOpenTable(OpenFile** table);
					//done
	void SystemTable_add(OpenFile *openFile);
					//done
	int SystemTable_del(OpenFile *openFile); 
					//done
	bool addTable(OpenFile *openFile, OpenFile **table = NULL);
					//done
	OpenFile** getThreadTable(int threadId);
					//done. recheck if we don't get what we want
	int Close(OpenFile* openFile, OpenFile** threadFileTable, int threadId);
					//done
	OpenFile *isAlreadyOpen(int sector);
					//done
	OpenFile *openFileSector_get(int sector);
					//done
};

#endif // FILESYS

#endif // FS_H
