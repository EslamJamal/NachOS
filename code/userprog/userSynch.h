

#ifndef SEMT_H
#define SEMT_H
typedef int sem_t;
#endif


#define SUCCESS true;
#define FAILED false;


extern sem_t do_UserSemaphoreCreate(char *name,int value);
extern bool do_UserSemaphoreDestroy(sem_t UserSemaphore);
extern void do_UserSemaphoreProberen(sem_t UserSemaphore);
extern void do_UserSemaphoreVerhogen(sem_t UserSemaphore);
