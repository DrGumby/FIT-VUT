#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <signal.h>
#include <time.h>


#define SEMAPHORE_NAME "/shared_memory"
#define MUTEX "/mutex"
#define ALL_ABOARD "/all_aboard"
#define RIDER_NAME "/rider_memory"
#define BUS_NAME "/bus_memory"
#define UNLOCKED 1

sem_t *semaphore = NULL;
sem_t *rider_sem = NULL;
sem_t *bus_sem = NULL;
sem_t *full_bus = NULL;
sem_t *mutex = NULL;
int *sharedCounter = NULL;
int sharedCounterId = 0;

int *transportedCount = NULL;
int transportedCountID = 0;

int *riderCount = NULL;
int riderCountID = 0;

int *busWaiting = NULL;
int busWaitingID = 0;


int setResources();

int min(int a, int b);

int argCheck(char* argument);

int main(int argc, char **argv)
{

    FILE* fp;
    fp = fopen("proj2.out","w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error, file cannot be opened\n");
    }
    srand(time(NULL));

    setbuf(fp, NULL);

    if(argc != 5)
    {
        fprintf(stderr, "Error, invalid arguments\n");
        return 1;
    }
    int R = argCheck(argv[1]);
    if(R == -1)
    {
        fprintf(stderr, "Invalid 'R' argument\n");
        return 1;
    }
    int C = argCheck(argv[2]);
    if(C == -1)
    {
        fprintf(stderr, "Invalid 'C' argument\n");
        return 1;
    }

    int ART = argCheck(argv[3]);
    if(ART == -1)
    {
        fprintf(stderr, "Invalid 'ART' argument\n");
        return 1;
    }

    int ABT = argCheck(argv[4]);
    if(ABT == -1)
    {
        fprintf(stderr, "Invalid 'ABT' argument\n");
        return 1;
    }



    if(setResources() == 1)
        return 1;

    int bus_time = rand() % ABT;

    pid_t processPid = fork();      //BUS
    if (processPid == 0) // child
    {
            int n = 0;
            sem_wait(semaphore);
            fprintf(fp, "%d: BUS: start\n", *sharedCounter);
            (*sharedCounter)++;
            sem_post(semaphore);
            while (*transportedCount < R)
            {
                sem_wait(semaphore);
                fprintf(fp, "%d: BUS: arrival\n", *sharedCounter);
                (*sharedCounter)++;

                sem_post(semaphore);
                sem_wait(semaphore);
                n = min(*busWaiting, C);
                fprintf(fp, "%d: BUS: start boarding: %d\n", *sharedCounter, *busWaiting);
                (*sharedCounter)++;
                for (int i = 0; i < n; i++) {
                    sem_post(rider_sem);
                    sem_wait(full_bus);
                    (*transportedCount)++;
                    (*busWaiting)--;
                }
                fprintf(fp, "%d: BUS: end boarding: %d\n",*sharedCounter, *busWaiting);
                (*sharedCounter)++;
                sem_post(semaphore);

                sem_wait(semaphore);
                fprintf(fp, "%d: BUS: depart\n", *sharedCounter);
                (*sharedCounter)++;
                sem_post(semaphore);


                usleep(bus_time*1000);

                sem_wait(semaphore);
                fprintf(fp, "%d: BUS: end\n", *sharedCounter);
                (*sharedCounter)++;
                sem_post(semaphore);


            }
            sem_wait(semaphore);
            fprintf(fp,"%d: BUS: finish\n", *sharedCounter);
            (*sharedCounter)++;

            sem_post(semaphore);
            exit(0);

    }
    else if(processPid > 0)
    {
        pid_t process2Pid = fork();     //RIDER auxiliary process
        if (process2Pid == 0) // child
        {
                int rider_spawn = 0;
                while (*riderCount < R)
                {

                    pid_t riderPid = fork();       //Rider
                    if (riderPid == 0)          //Child
                    {
                        sem_wait(bus_sem);
                        (*riderCount)++;
                        int ridNum = *riderCount;
                        fprintf(fp,"%d: RID %d: start\n", *sharedCounter, ridNum);
                        (*sharedCounter)++;
                        (*busWaiting)++;

                        fprintf(fp, "%d: RID %d: enter: %d\n", *sharedCounter, ridNum, *busWaiting);
                        (*sharedCounter)++;

                        sem_post(bus_sem);

                        sem_wait(rider_sem);
                        sem_wait(bus_sem);
                        fprintf(fp, "%d: RID %d: boarding\n", *sharedCounter, ridNum);
                        (*sharedCounter)++;
                        sem_post(bus_sem);
                        sem_post(full_bus);

                        exit(0);


                    }
                    else if(riderPid > 0)
                    {
                        rider_spawn = rand() % ART;
                        usleep(rider_spawn*1000);




                    }

                }

                exit(0);
                //printf("Exited while\n");
            }

        waitpid(process2Pid, NULL, 0);
        //printf("Waiting donw\n");

        }


    waitpid(processPid, NULL, 0);
    //printf("Waiting donw2\n");
    shmctl(sharedCounterId, IPC_RMID, NULL);
    shmctl(riderCountID, IPC_RMID, NULL);
    shmctl(busWaitingID, IPC_RMID, NULL);
    shmctl(transportedCountID, IPC_RMID, NULL);
    sem_close(semaphore);
    sem_unlink(SEMAPHORE_NAME);
    sem_close(rider_sem);
    sem_unlink(RIDER_NAME);
    sem_close(bus_sem);
    sem_unlink(BUS_NAME);
    sem_close(full_bus);
    sem_unlink(ALL_ABOARD);
    sem_close(mutex);
    sem_unlink(MUTEX);
    return 0;
}

int setResources()
{




    if ((sharedCounterId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT  | 0666)) == -1)      //Create shared counter
    {
        fprintf(stderr, "Error, shared memory allocation failed\n");
        return 1;
    }
    if ((sharedCounter = (int *)shmat(sharedCounterId, NULL, 0)) == NULL)           //Attach shared counter
    {
        fprintf(stderr, "Error, shared memory attachment failed\n");
        return 1;
    }
    if ((riderCountID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT  | 0666)) == -1)      //Create shared counter
    {
        fprintf(stderr, "Error, shared memory allocation failed\n");
        return 1;
    }
    if ((riderCount = (int *)shmat(riderCountID, NULL, 0)) == NULL)           //Attach shared counter
    {
        fprintf(stderr, "Error, shared memory attachment failed\n");
        return 1;
    }

    if ((busWaitingID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT  | 0666)) == -1)      //Create shared counter
    {
        fprintf(stderr, "Error, shared memory allocation failed\n");
        return 1;
    }
    if ((busWaiting = (int *)shmat(busWaitingID, NULL, 0)) == NULL)           //Attach shared counter
    {
        fprintf(stderr, "Error, shared memory attachment failed\n");
        return 1;
    }
    if ((transportedCountID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT  | 0666)) == -1)      //Create shared counter
    {
        fprintf(stderr, "Error, shared memory allocation failed\n");
        return 1;
    }
    if ((transportedCount = (int *)shmat(transportedCountID, NULL, 0)) == NULL)           //Attach shared counter
    {
        fprintf(stderr, "Error, shared memory attachment failed\n");
        return 1;
    }

    //Initialize shared memory items

    *sharedCounter = 1;
    *riderCount = 0;
    *busWaiting = 0;
    *transportedCount = 0;

    //Open semaphores

    if ((semaphore = sem_open(SEMAPHORE_NAME, O_CREAT | O_EXCL, 0666, UNLOCKED)) == SEM_FAILED)
    {
        fprintf(stderr, "Error, failed to open semaphore\n");
        return 1;
    }
    if ((rider_sem = sem_open(RIDER_NAME, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        fprintf(stderr, "Error, failed to open semaphore\n");
        return 1;
    }

    if ((bus_sem = sem_open(BUS_NAME, O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
    {
        fprintf(stderr, "Error, failed to open semaphore\n");
        return 1;
    }
    if ((mutex = sem_open(MUTEX, O_CREAT | O_EXCL, 0666, UNLOCKED)) == SEM_FAILED)
    {
        fprintf(stderr, "Error, failed to open semaphore\n");
        return 1;
    }
    if ((full_bus = sem_open(ALL_ABOARD, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        fprintf(stderr, "Error, failed to open semaphore\n");
        return 1;
    }
    return 0;
}

int min(int a, int b)
{
    if (a < b)
        return a;
    else
        return b;
}

int argCheck(char * argument)
{
    char* endptr = NULL;
    int retval = 0;
    retval = (int)strtol(argument, &endptr, 0);
    if(*endptr != '\0')
        return -1;
    else
        return retval;
}
