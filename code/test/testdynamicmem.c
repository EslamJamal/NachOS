
#include "syscall.h"
// #include "dynamicmem.h"


#ifndef   	_MEM_ALLOC_TYPES_H_
#define   	_MEM_ALLOC_TYPES_H_



/* Structure declaration for a free block */
typedef struct mem_free_block{
    int size;
    struct mem_free_block *next;
} mem_free_block_t; 


/* Specific metadata for used blocks */
typedef struct mem_used_block{
    int size;
} mem_used_block_t;

#endif


/* pointer to the beginning of the memory region to manage */
void* heap_start;

/* Pointer to the first free block in the heap */
mem_free_block_t *first_free = 0; 
static int memory_size;

#define ULONG(x)((long unsigned int)(x))


void memory_init(int size)
{

    // heap_start = my_mmap(memory_size);
    heap_start = (void*) Sbrk(size);
    PutString("heap start:");
    PutInt((int) heap_start);
    PutChar('\n');
    if(heap_start ==(void*) -1){
        PutString("requested size is mpre than the availabe memory for a process\n");
        PutString("closing the process...\n");
        Halt();
    }
    first_free = (mem_free_block_t*)heap_start;
    first_free->size = size;
    first_free->next = 0;

}


void *memory_alloc(int size)
{
    /* TODO: insert your code here */   

    //allocated mem address
    void *requested_mem_add ;

    //init meta data, freeblocks
    mem_used_block_t *meta_data;
    mem_free_block_t *tmp_free = first_free;
    mem_free_block_t *tmp_before;

    //size check
    if(size < memory_size) 
    {      
        while (tmp_free != 0)
        {
            if(first_free->next == 0 || first_free->size > (size+(int) sizeof(mem_used_block_t*)))
            {
                //allocate at first block    
                int tempsize = first_free->size;
                meta_data =  (mem_used_block_t*) first_free;
                meta_data->size = size;
                requested_mem_add = (mem_used_block_t*) first_free + 1;

                first_free = ((mem_free_block_t*)((char*) first_free + size + (sizeof(mem_used_block_t*))));
                first_free->size = tempsize - (size + sizeof(mem_used_block_t*));
                first_free->next = tmp_free->next;    

                //if there is space after after allocated block
                if (first_free->size <= 16)
                {
                    first_free = first_free->next;
                }
                break;
            }
            else if(tmp_free->size  > (size+ (int )sizeof(mem_used_block_t*)))
            {
                //allocate to blocks after first block
                int tempsize = tmp_free->size;
                meta_data =  (mem_used_block_t*) tmp_free;
                meta_data->size = size;
                requested_mem_add = (mem_used_block_t*) tmp_free + 1;

                //new allocation var
                mem_free_block_t *next_free;
                next_free = (mem_free_block_t*) ((char*)tmp_free + size + (sizeof(mem_used_block_t*)));
                next_free->size = tempsize - (size + sizeof(mem_used_block_t*));
                tmp_before->next = next_free;
                
                //pointers
                if(tmp_free->next == 0)
                {
                    next_free->next = 0;
                }
                else
                {
                    next_free->next = tmp_free->next;
                }
                if(first_free->next == tmp_free)
                {
                    first_free->next = next_free;
                } // PutString("requested size is more than free memory available\n");
        // PutString("closing the process...\n");
        // Halt();

                //if there is space after after allocated block
                if(next_free->size<=16)
                {
                    tmp_before->next = next_free->next;
                }
                break;
            }
            tmp_before = tmp_free;
            tmp_free = (mem_free_block_t*) tmp_free->next;
        }
    }
    else
    {
        // printf("Not enough spaceee!");
        // PutString("requested size is more than free memory available\n");
        // PutString("closing the process...\n");
        // Halt();
        return 0;
    }

    return requested_mem_add;
}




void memory_free(void *p)
{
    /* TODO: insert your code here */

    mem_free_block_t *mem_addr = (mem_free_block_t*)((char*) p - sizeof(mem_used_block_t*)); 
    int freesize = (int) mem_addr->size;
    mem_free_block_t *mem_next;
    
    if (first_free == 0)
    {
        first_free = mem_addr;
        first_free->size = freesize;
        first_free->next = 0;
    }
    else if(mem_addr < first_free)
    {
        mem_next = first_free;   
        first_free = mem_addr;
        first_free->size = freesize+sizeof(mem_used_block_t*);
        first_free->next = mem_next;
    }
    else
    {
        mem_next = mem_addr;
        mem_next->size = freesize+sizeof(mem_used_block_t*);   
        mem_free_block_t *mem_before;

        if(mem_next > first_free->next)
        {
            mem_before = first_free;
            while (mem_before != 0)
            {
                if (mem_before->next > mem_addr )
                {
                    break;
                }
                mem_before = mem_before->next;
            
            }
            mem_next->next = mem_before->next;
            mem_before->next = mem_next;
        }
        else
        {
            
            mem_next->next = first_free->next;
            first_free->next = mem_next;
            
        }
    }

    //check coallesce
    mem_free_block_t *coal_temp = first_free;
    int countcoal = 0;
    while (coal_temp != 0)
    {
        countcoal++;
        if(coal_temp->size + (int)((char*)coal_temp - (char*) heap_start) ==  (int) ((char*)coal_temp->next -(char*) heap_start))
        {
            mem_free_block_t *tmp_next_coal = coal_temp->next;
            mem_free_block_t *next_coal = coal_temp->next;
            
            int firstsize = coal_temp->size;
            int secondsize = tmp_next_coal->size;

            tmp_next_coal = coal_temp;
            tmp_next_coal->size = firstsize + secondsize;
            tmp_next_coal->next = next_coal->next;
            
            coal_temp = tmp_next_coal;
        }
        else
        {
            coal_temp = (mem_free_block_t*)coal_temp->next;
        }
    }

}


int main(){

  /* The main can be changed, it is *not* involved in tests */
  memory_init(10);
  // for( i = 0; i < 10; i++){
    int *b =(int*) memory_alloc(10);
    // int add = *(b);
    PutInt((int) b);
    PutChar('\n');
    // memory_free((void*) add);
    int i;
    for(i = 0;i<10;i++){
      b[i] = i;
      PutInt(b[i]);
      PutChar('\n');
    }  
 


  Halt();
  /* not reached */
    return 0;
}