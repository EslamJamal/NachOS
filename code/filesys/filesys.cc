// filesys.cc 
//	Routines to manage the overall operation of the file system.
//	Implements routines to map from textual file names to files.
//
//	Each file in the file system has:
//	   A file header, stored in a sector on disk 
//		(the size of the file header data structure is arranged
//		to be precisely the size of 1 disk sector)
//	   A number of data blocks
//	   An entry in the file system directory
//
// 	The file system consists of several data structures:
//	   A bitmap of free disk sectors (cf. bitmap.h)
//	   A directory of file names and file headers
//
//      Both the bitmap and the directory are represented as normal
//	files.  Their file headers are located in specific sectors
//	(sector 0 and sector 1), so that the file system can find them 
//	on bootup.
//
//	The file system assumes that the bitmap and directory files are
//	kept "open" continuously while Nachos is running.
//
//	For those operations (such as Create, Remove) that modify the
//	directory and/or bitmap, if the operation succeeds, the changes
//	are written immediately back to disk (the two files are kept
//	open during all this time).  If the operation fails, and we have
//	modified part of the directory and/or bitmap, we simply discard
//	the changed version, without writing it back to disk.
//
// 	Our implementation at this point has the following restrictions:
//
//	   there is no synchronization for concurrent accesses
//	   files have a fixed size, set when the file is created
//	   files cannot be bigger than about 3KB in size
//	   there is no hierarchical directory structure, and only a limited
//	     number of files can be added to the system
//	   there is no attempt to make the system robust to failures
//	    (if Nachos exits in the middle of an operation that modifies
//	    the file system, it may corrupt the disk)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "stdlib.h"
#include "disk.h"
#include "bitmap.h"
#include "directory.h"
#include "filehdr.h"
#include "filesys.h"

// Sectors containing the file headers for the bitmap of free sectors,
// and the directory of files.  These file headers are placed in well-known 
// sectors, so that they can be located on boot-up.
#define FreeMapSector 		0
#define DirectorySector 	1

// Initial file sizes for the bitmap and directory; until the file system
// supports extensible files, the directory size sets the maximum number 
// of files that can be loaded onto the disk.
#define FreeMapFileSize 	(NumSectors / BitsInByte)
#define NumDirEntries 		10
#define DirectoryFileSize 	(sizeof(DirectoryEntry) * NumDirEntries)

//----------------------------------------------------------------------
// FileSystem::FileSystem
// 	Initialize the file system.  If format = TRUE, the disk has
//	nothing on it, and we need to initialize the disk to contain
//	an empty directory, and a bitmap of free sectors (with almost but
//	not all of the sectors marked as free).  
//
//	If format = FALSE, we just have to open the files
//	representing the bitmap and the directory.
//
//	"format" -- should we initialize the disk?
//----------------------------------------------------------------------

FileSystem::FileSystem(bool format)
{
    this->createPerThreadTable();

    DEBUG('f', "Initializing the file system.\n");
    if (format) {
        BitMap *freeMap = new BitMap(NumSectors);
        Directory *directory = new Directory(NumDirEntries);
	    FileHeader *mapHdr = new FileHeader;
	    FileHeader *dirHdr = new FileHeader;

        DEBUG('f', "Formatting the file system.\n");

        // First, allocate space for FileHeaders for the directory and bitmap
        // (make sure no one else grabs these!)
	    freeMap->Mark(FreeMapSector);	    
	    freeMap->Mark(DirectorySector);

        // Second, allocate space for the data blocks containing the contents
        // of the directory and bitmap files.  There better be enough space!

	    ASSERT(mapHdr->Allocate(freeMap, FreeMapFileSize));
	    ASSERT(dirHdr->Allocate(freeMap, DirectoryFileSize));

        // Flush the bitmap and directory FileHeaders back to disk
        // We need to do this before we can "Open" the file, since open
        // reads the file header off of disk (and currently the disk has garbage
        // on it!).

        DEBUG('f', "Writing headers back to disk.\n");
	    mapHdr->WriteBack(FreeMapSector);
        dirHdr->type = dir;
	    dirHdr->WriteBack(DirectorySector);

        // Add the directories . and ..
        directory->Add(".", DirectorySector);
        directory->Add("..", DirectorySector);
        // OK to open the bitmap and directory files now
        // The file system operations assume these two files are left open
        // while Nachos is running.

        freeMapFile = new OpenFile(FreeMapSector);
        this->PerThreadTable->OpenTable[ROOT_DIR] = new OpenFile(DirectorySector);
        this->PerThreadTable->OpenTable[CURR_DIR] = this->PerThreadTable->OpenTable[ROOT_DIR];
     
        // Once we have the files "open", we can write the initial version
        // of each file back to disk.  The directory at this point is completely
        // empty; but the bitmap has been changed to reflect the fact that
        // sectors on the disk have been allocated for the file headers and
        // to hold the file data for the directory and bitmap.

        DEBUG('f', "Writing bitmap and directory back to disk.\n");
	    freeMap->WriteBack(freeMapFile);	 // flush changes to disk
	    directory->WriteBack(this->PerThreadTable->OpenTable[CURR_DIR]);

	    if (DebugIsEnabled('f')) {
	        freeMap->Print();
	        directory->Print();

            delete freeMap; 
	        delete directory; 
	        delete mapHdr; 
	        delete dirHdr;
	    }
    } else {
        // if we are not formatting the disk, just open the files representing
        // the bitmap and directory; these are left open while Nachos is running
        freeMapFile = new OpenFile(FreeMapSector);
        this->PerThreadTable->OpenTable[ROOT_DIR] = new OpenFile(DirectorySector);
        this->PerThreadTable->OpenTable[CURR_DIR] = this->PerThreadTable->OpenTable[ROOT_DIR];
    }
}
void FileSystem::createPerThreadTable(){
    this->PerThreadTable = (table_t*) malloc(sizeof(table_t));
    int tablesize = sizeof(int) * MAX_OPEN_FILE;
    this->PerThreadTable->OpenTable = (OpenFile**) malloc(tablesize);
    this->PerThreadTable->threadId = 0;
    this->createOpenTable(this->PerThreadTable->OpenTable);
    this->PerThreadTable->next = NULL;
}
//----------------------------------------------------------------------
// FileSystem::Create
// 	Create a file in the Nachos file system (similar to UNIX create).
//	Since we can't increase the size of files dynamically, we have
//	to give Create the initial size of the file.
//
//	The steps to create a file are:
//	  Make sure the file doesn't already exist
//        Allocate a sector for the file header
// 	  Allocate space on disk for the data blocks for the file
//	  Add the name to the directory
//	  Store the new file header on disk 
//	  Flush the changes to the bitmap and the directory back to disk
//
//	Return TRUE if everything goes ok, otherwise, return FALSE.
//
// 	Create fails if:
//   		file is already in directory
//	 	no free space for file header
//	 	no free entry for file in directory
//	 	no free space for data blocks for the file 
//
// 	Note that this implementation assumes there is no concurrent access
//	to the file system!
//
//	"name" -- name of file to be created
//	"initialSize" -- size of file to be created
//----------------------------------------------------------------------

bool
FileSystem::Create(const char *name, int initialSize)
{
    Directory *directory;
    BitMap *freeMap;
    FileHeader *hdr;
    int sector;
    bool success;

    DEBUG('f', "Creating file %s, size %d\n", name, initialSize);

    directory = new Directory(NumDirEntries);
    directory->FetchFrom(this->PerThreadTable->OpenTable[CURR_DIR]);

    if (directory->Find(name) != -1)
      success = false;			// file is already in directory
    else {	
        freeMap = new BitMap(NumSectors);
        freeMap->FetchFrom(freeMapFile);
        sector = freeMap->Find();	// find a sector to hold the file header
    	if (sector == -1) 		
            success = false;		// no free block for file header 
        else if (!directory->Add(name, sector))
            success = false;	// no space in directory
	else {
    	    hdr = new FileHeader;
	    if (!hdr->Allocate(freeMap, initialSize))
            	success = false;	// no space on disk for data
	    else {	
	    	success = true;
		// everthing worked, flush all changes back to disk
    	    	hdr->WriteBack(sector); 		
    	    	directory->WriteBack(this->PerThreadTable->OpenTable[CURR_DIR]);
    	    	freeMap->WriteBack(freeMapFile);
	    }
            delete hdr;
	}
        delete freeMap;
    }
    delete directory;
    return success;
}

//----------------------------------------------------------------------
// FileSystem::Open
// 	Open a file for reading and writing.  
//	To open a file:
//	  Find the location of the file's header, using the directory 
//	  Bring the header into memory
//
//	"name" -- the text name of the file to be opened
//----------------------------------------------------------------------

OpenFile *
FileSystem::Open(const char *name, int threadId)
{ 
    Directory *directory = new Directory(NumDirEntries);
    OpenFile *openFile = NULL;
    int sector;

    DEBUG('f', "Opening file %s\n", name);
    directory->FetchFrom(this->PerThreadTable->OpenTable[CURR_DIR]);
    sector = directory->Find(name); 
    if (sector >= 0){
        openFile = this->isAlreadyOpen(sector);
        bool flag = false;
        if (openFile == NULL){
            openFile = new OpenFile(sector);	// name was found in directory
            flag = true;
        }
    openFile->seekPosition_add(threadId);
    OpenFile** Table = this->getThreadTable(threadId);
    if (threadId != 0){
        this->SystemTable_add(openFile);
    }
	if (!this->addTable(openFile, Table)){
        if (flag){
            delete openFile;
        }
        delete directory;
        return NULL;
    }
    delete directory;
    }
    return openFile;				// return NULL if not found
}

//----------------------------------------------------------------------
// FileSystem::Remove
// 	Delete a file from the file system.  This requires:
//	    Remove it from the directory
//	    Delete the space for its header
//	    Delete the space for its data blocks
//	    Write changes to directory, bitmap back to disk
//
//	Return TRUE if the file was deleted, FALSE if the file wasn't
//	in the file system.
//
//	"name" -- the text name of the file to be removed
//----------------------------------------------------------------------

bool
FileSystem::Remove(const char *name, int threadId)
{ 
    Directory *directory;
    BitMap *freeMap;
    FileHeader *fileHdr;
    int sector;
    
    directory = new Directory(NumDirEntries);
    directory->FetchFrom(this->PerThreadTable->OpenTable[ROOT_DIR]);
    sector = directory->Find(name);
    if (sector == -1) {
       delete directory;
       return FALSE;			 // file not found 
    }
    fileHdr = new FileHeader;
    fileHdr->FetchFrom(sector);

    freeMap = new BitMap(NumSectors);
    freeMap->FetchFrom(freeMapFile);

    fileHdr->Deallocate(freeMap);  		// remove data blocks
    freeMap->Clear(sector);			// remove header block
    directory->Remove(name);

    freeMap->WriteBack(freeMapFile);		// flush to disk
    directory->WriteBack(this->PerThreadTable->OpenTable[ROOT_DIR]);        // flush to disk
    delete fileHdr;
    delete directory;
    delete freeMap;
    return TRUE;
} 

//----------------------------------------------------------------------
// FileSystem::List
// 	List all the files in the file system directory.
//----------------------------------------------------------------------

void
FileSystem::List()
{
    Directory *directory = new Directory(NumDirEntries);

    directory->FetchFrom(this->PerThreadTable->OpenTable[CURR_DIR]);
    directory->List();
    delete directory;
}

//----------------------------------------------------------------------
// FileSystem::Print
// 	Print everything about the file system:
//	  the contents of the bitmap
//	  the contents of the directory
//	  for each file in the directory,
//	      the contents of the file header
//	      the data in the file
//----------------------------------------------------------------------

void
FileSystem::Print()
{
    FileHeader *bitHdr = new FileHeader;
    FileHeader *dirHdr = new FileHeader;
    BitMap *freeMap = new BitMap(NumSectors);
    Directory *directory = new Directory(NumDirEntries);

    printf("Bit map file header:\n");
    bitHdr->FetchFrom(FreeMapSector);
    bitHdr->Print();

    printf("Directory file header:\n");
    dirHdr->FetchFrom(DirectorySector);
    dirHdr->Print();

    freeMap->FetchFrom(freeMapFile);
    freeMap->Print();

    directory->FetchFrom(this->PerThreadTable->OpenTable[CURR_DIR]);
    // directory->Print();

    delete bitHdr;
    delete dirHdr;
    delete freeMap;
    delete directory;
} 

FileSystem::~FileSystem(){

    table_t* queue = PerThreadTable;
    while(queue != NULL){
        for(int i = 0 ; i < MAX_OPEN_FILE ; i++)
            delete queue->OpenTable[i];
        queue = queue->next;
        delete PerThreadTable;
        PerThreadTable = queue;
    }
    delete queue;

    system_table_t* gqueue = SystemTable;
    while(gqueue != NULL){
        delete gqueue->openFile;
        gqueue = gqueue->next;
        delete SystemTable;
        SystemTable = gqueue;
    }
    delete gqueue;

    delete freeMapFile;
}

void FileSystem::GetInTable(int* table, int threadId){
    table_t * Table = this->PerThreadTable;
    while(Table->next != NULL){
        Table = Table->next;
    }
    Table->next = (table_t*) malloc(sizeof(table_t));
    Table->next->OpenTable = (OpenFile**) malloc(sizeof(OpenFile*) * MAX_OPEN_FILE);
    this->createOpenTable(Table->next->OpenTable);
    Table->next->threadId = threadId;
    Table->next->next = NULL;
}

bool FileSystem::GetOutTable(int threadId){
    table_t* Table = this->PerThreadTable;
    while(Table->next != NULL && Table->next->threadId != threadId){
        Table = Table->next;
    }
    if (Table->next == NULL){
        return false;
    }
    for (int k = 0; k < MAX_OPEN_FILE; k++){
        if(Table->next->OpenTable[k] != NULL)
            this->Close(Table->next->OpenTable[k],Table->next->OpenTable,threadId);
    }
    table_t* temp = Table->next;
    delete Table->next;
    Table->next = temp->next;
    return true;
}

OpenFile* FileSystem::isAlreadyOpen(int sector){
    system_table_t* systable = this->SystemTable;
    while (systable != NULL){
        if (systable->openFile->sector_get() == sector){
            return systable->openFile;
        }
        systable = systable->next;
    }
    return NULL;
}

void FileSystem::SystemTable_add(OpenFile* openFile){
    system_table_t* systable = this->SystemTable;
    
    if (systable == NULL){ //if empty, add
        this->SystemTable = (system_table_t* ) malloc(sizeof(system_table_t));
        this->SystemTable->openFile = openFile;
        this->SystemTable->next = NULL;
    }
    else{
        while(systable->next != NULL){ //if not empty, go at the end then add
        systable = systable->next;
        }
        systable->next = (system_table_t* ) malloc(sizeof(system_table_t));
        this->SystemTable->openFile = openFile;
        this->SystemTable->next = NULL;
    }
}

int FileSystem::SystemTable_del(OpenFile* openFile){
    system_table_t* systable = this->SystemTable;
    
    if (systable == NULL){ //if empty, can't remove
        return 0;
    }
    if (systable->openFile == openFile){
        system_table_t* temp = systable->next;
        delete systable;
        this->SystemTable = temp;
        return 1;
    }

    while(systable->next != NULL && systable->next->openFile != openFile){ //if not empty, search the openFile
        systable = systable->next;
    }
    if (systable->next == NULL){ //if not found
        return 0;
    }
    else{ //if found, remove
        system_table_t* temp = systable->next->next;
        delete systable->next;
        systable->next = temp;
        return 1;
    }
}

OpenFile** FileSystem::getThreadTable(int threadId){
    table_t* threads = this->PerThreadTable;

    while(threads != NULL){
        if (threads->threadId == threadId){
            return threads->OpenTable;
        }
        threads = threads->next;
    }
    exit(-1);
    if (threads->threadId != threadId){
        abort();
    }
    return NULL;
}

void FileSystem::createOpenTable(OpenFile** table){
    int i;
    for (i = 0; i < MAX_OPEN_FILE; i++){
        table[i] = NULL;
    }
}

bool FileSystem::ChangeDirectory(const char *name, int threadId){
    OpenFile *openFile;
    OpenFile **Table = this->getThreadTable(threadId);
    if (strcmp(name, "/") == 0){
        Table[CURR_DIR] = NULL;
        Table[CURR_DIR] = Table[ROOT_DIR];
        return true;
    }
    if ((openFile = Open(name)) == NULL){
        return false;
    }
    if (!openFile->isDir()){
        return false;
    }
    Table[CURR_DIR] = NULL;
    Table[CURR_DIR] = openFile;
    return true;
}

bool FileSystem::MakeDirectory(const char *name, int threadId){
    BitMap *freeMap;
    Directory *curr_dir;
    Directory *directory;
    OpenFile *openFile;
    FileHeader *hdr;
    int sector;
    hdr = new FileHeader;
    freeMap = new BitMap(NumSectors);
    freeMap->FetchFrom(freeMapFile);
    sector = freeMap->Find();
    curr_dir = new Directory(NumDirEntries);
    curr_dir->FetchFrom(this->PerThreadTable->OpenTable[CURR_DIR]);

    if(strcmp(name, "/") == 0){
        return false;
    }
    
    if (curr_dir->Find(name) != -1){
        return false;
    }

    if (sector == -1){
        return false;
    }

    if (!curr_dir->Add(name, sector)){
        return false;
    }

    if (!hdr->Allocate(freeMap, DirectoryFileSize)){
        return false;
    }

    directory = new Directory(NumDirEntries);
    directory->Add(".", sector);
    directory->Add("..", curr_dir->Find("."));
    hdr->type = dir;
    hdr->WriteBack(sector);
    freeMap->WriteBack(freeMapFile);
    openFile = new OpenFile(sector);
    directory->WriteBack(openFile);
    curr_dir->WriteBack(this->PerThreadTable->OpenTable[CURR_DIR]);

    delete curr_dir;
    delete openFile;
    delete directory;
    delete hdr;
    delete freeMap;
    return true;
}

bool FileSystem::RemoveDirectory(const char *name, int threadId){
    if ((!strcmp(name, "/")) || (!strcmp(name, ".")) || (!strcmp(name, ".."))){ // Can't delete basic dirs
        return false;
    }
    BitMap *freeMap;
    Directory *curr_dir;
    Directory *directory;
    OpenFile *openFile;
    FileHeader *hdr;
    int sector;

    curr_dir = new Directory(NumDirEntries);
    curr_dir->FetchFrom(this->PerThreadTable->OpenTable[CURR_DIR]);
    sector = curr_dir->Find(name);
    if (sector == -1){
        return false;
    }
    hdr = new FileHeader;
    hdr->FetchFrom(sector);

    directory = new Directory(NumDirEntries);
    openFile = this->openFileSector_get(sector);

    if (!openFile){
        openFile = new OpenFile(sector);
    }
    directory->FetchFrom(openFile);
    
    if (!directory->isEmpty()){
        delete directory;
        delete hdr;
        delete openFile;
        return false;
    }
    freeMap = new BitMap(NumSectors);
    freeMap->FetchFrom(freeMapFile);    
    hdr->Deallocate(freeMap);
    freeMap->Clear(sector);
    curr_dir->Remove(name);
    freeMap->WriteBack(freeMapFile);
    curr_dir->WriteBack(this->PerThreadTable->OpenTable[CURR_DIR]);

    delete hdr;
    delete freeMap;
    delete curr_dir;
    delete directory;
    delete openFile;
    return true;
}

OpenFile* FileSystem::openFileSector_get(int sector){
    system_table_t* systable = this->SystemTable;
    while (systable != NULL){
        if (systable->openFile->sector_get() == sector){
            return systable->openFile;
        }
        systable = systable->next;
    }
    return NULL;
}

int FileSystem::Close(OpenFile* openFile, OpenFile** threadTable, int threadId){
    if (openFile->alreadyOpened()){
        for (int i = 0; i < MAX_OPEN_FILE; i++){
            if (threadTable[i] == openFile){
                threadTable[i] = NULL;
            }
        }
        openFile->seekPosition_del(threadId);
        return true;
    }
    for (int i = 0; i < MAX_OPEN_FILE; i++){
        if (threadTable[i] == openFile){
            delete threadTable[i];
            return true;
        }
    }
    return false;
}

bool FileSystem::addTable(OpenFile *openFile, OpenFile **table){
    for (int i = 0; i < MAX_OPEN_FILE; i++){
        if (table[i] == NULL){
            table[i] = openFile;
            return true;
        }
    }
    return false;
}