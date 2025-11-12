#ifndef __PROCESS__
#define __PROCESS__

#include <stdbool.h>

// Process
// Ready: The process is in the queue waiting for the scheduler.
// Running: The process is consuming CPU
// Bloqued: The process is doing input/output operations.
// Finished: The process finishes its work.
enum processStatus{Ready, Running, Bloqued, Finished};



typedef struct _process
{
    // Information required by the scheduler
    int id;
    char* name;
    int burst; // [[2,Running],[],[burst,operation]] 
    int priority;
    int arrive_time;           
    // Information obtained during and after the life of the process
    int* lifecycle;     
    int waiting_time;  //Temps espera    
    int return_time;   //Temps de retorn      
    int response_time; //Temps de resposta
    bool completed;         
} Process;

Process initProcess(int id, char* name, int burst, int priority, int arrive_time);
Process initProcessFromTokens(char* line, char* separator);
void destroyProcess(Process p);
void printProcess(Process proces);
int compareArrival(const void *s1, const void *s2);
int compareBurst(const void *s1, const void *s2);
int comparePriority(const void *s1, const void *s2);
void printLife(Process* p, int t);

#endif



