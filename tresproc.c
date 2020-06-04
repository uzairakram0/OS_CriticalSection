#include <unistd.h>     //unix API posix
#include <stdio.h>      //standard I/O file stream/ I/O streams
#include <stdlib.h>     //standard library for c
#include <sys/types.h>  //datatypes in system size_t
#include <sys/ipc.h>    //interprocess communication: meassages/semaphores/shared_mem
#include <sys/shm.h>    //shared memory facility
#include <sys/wait.h>   //for wait call
#include <sys/stat.h>   //contains constructs to get info about file atributes
#include <fcntl.h>      //file control options
#include <semaphore.h>  //posix semaphores

#define file "F.txt"
#define size 8

/* Shaared memory for coordinating processes in the critical section */
struct shared_mem{
    sem_t sem;
    int N;
    FILE *fptr;
};//End shared_mem struct

int main(){
    
    //Initialize shared memory
    int shmID;
    struct shared_mem *shared;
    //Gets the offset/ID for the shared memory location
    shmID  = shmget(IPC_PRIVATE, sizeof(*shared), IPC_CREAT | 0644);
    //Maps the shared memory struct to the shared memory
    shared  = shmat(shmID, 0, 0);
    //initialize the integer N to 0
    shared->N = 0;
    
    //Initialize semaphore
    /* sem_init(sem_t *sem, int pshared, unsigned int value);
    sem:specifies the semaphore to initialize
    pshared: non-zero means process-shared semaphore, 0 = thread shared semahore
    value: initial value of the semaphore */
    sem_init(&shared->sem, 1, 1);
    
    
    //CREATING SYSTEM OF 3 processes
    int status = 0;
    pid_t pid = fork();
    if(pid > 0) { fork(); }
    
    for(int i = 0; i < 200; i++){
        
        //Wait semaphore: protect critical section/ lock semaphore
        /* int sem_wait(sem_t *sem); 
        decrements when the value > 0
        and blocks when value == 0 */
        sem_wait(&shared->sem);
        
        /*Read the integer from file*/
        //Open F
        shared->fptr = fopen(file, "r");
        //Read the integer N from the file
        fscanf(shared->fptr, "%d", &shared->N);
        //Close F
        fclose(shared->fptr);
        
        
        //Output N and the process PID (either on screen or test file)
        printf("\npid:%d\tN = %d", getpid(), shared->N);
        //Increment N by 1
        shared->N = shared->N + 1;
        
        
        /*Write to file*/
        //Open F
        shared->fptr = fopen(file, "w");
        //Write N to F (overwriting the current value in F)
        fprintf(shared->fptr, "%d", shared->N);
        //Close F
        fclose(shared->fptr);
        
        //Post the semaphore: release critical section/ signal release semaphore
        /* int sem_post(sem_t *sem); 
        Increments semaphore then, if the semaphore's value becomes greater than 0 then
        another thread or process blocked on sem_wait will be woken up and proceed to lock
        the semaphore*/
        sem_post(&shared->sem);
    }
    
    //Wait for the child processes to finish
    while ((pid = wait(&status)) > 0);
    if (pid > 0) printf("\n");
    
    exit(0);
    
}
