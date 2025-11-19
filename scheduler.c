#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "process.h"
#include "queue.h"
#include "scheduler.h"

int num_algorithms() {
  return sizeof(algorithmsNames) / sizeof(char *);
}

int num_modalities() {
  return sizeof(modalitiesNames) / sizeof(char *);
}

size_t initFromCSVFile(char* filename, Process** procTable){
    FILE* f = fopen(filename,"r");
    
    size_t procTableSize = 10;
    
    *procTable = malloc(procTableSize * sizeof(Process));
    Process * _procTable = *procTable;

    if(f == NULL){
      perror("initFromCSVFile():::Error Opening File:::");   
      exit(1);             
    }

    char* line = NULL;
    size_t buffer_size = 0;
    size_t nprocs= 0;
    while( getline(&line,&buffer_size,f)!=-1){
        if(line != NULL){
            Process p = initProcessFromTokens(line,";");

            if (nprocs==procTableSize-1){
                procTableSize=procTableSize+procTableSize;
                _procTable=realloc(_procTable, procTableSize * sizeof(Process));
            }

            _procTable[nprocs]=p;

            nprocs++;
        }
    }
   free(line);
   fclose(f);
   return nprocs;
}

size_t getTotalCPU(Process *procTable, size_t nprocs){
    size_t total=0;
    for (int p=0; p<nprocs; p++ ){
        total += (size_t) procTable[p].burst;
    }
    return total;
}

int getCurrentBurst(Process* proc, int current_time){
    int burst = 0;
    for(int t=0; t<current_time; t++){
        if(proc->lifecycle[t] == Running){
            burst++;
        }
    }
    return burst;
}

int run_dispatcher(Process *procTable, size_t nprocs, int algorithm, int modality, int quantum){

    //Process * _proclist;

    qsort(procTable,nprocs,sizeof(Process),compareArrival);

    init_queue();
    size_t duration = getTotalCPU(procTable, nprocs) +1;

    for (int p=0; p<nprocs; p++ ){
        procTable[p].lifecycle = malloc( duration * sizeof(int));
        for(int t=0; t<duration; t++){
            procTable[p].lifecycle[t]=-1;
        }
        procTable[p].waiting_time = 0;
        procTable[p].return_time = 0;
        procTable[p].response_time = 0;
        procTable[p].completed = false;
    }
    int time = 0;
    int completed = 0;
    Process *current = NULL;

    while (completed < nprocs) {

        for (int p = 0; p < nprocs; p++) {
            if (procTable[p].arrive_time == time) {
                enqueue(&procTable[p]);
            }
        }

        if (current == NULL) {
            current = dequeue();

            if (current != NULL && current->response_time == 0) {
                current->response_time = time - current->arrive_time;
            }
        }

        for (int p = 0; p < nprocs; p++) {
            if (&procTable[p] == current) {
                procTable[p].lifecycle[time] = Running;
            }
            else if (procTable[p].completed) {
                procTable[p].lifecycle[time] = Finished;
            }
            else if (procTable[p].arrive_time <= time) {
                procTable[p].lifecycle[time] = Bloqued;
            }
            else {
                procTable[p].lifecycle[time] = -1; 
            }
        }

        if (current != NULL) {
            int burstUsed = getCurrentBurst(current, time + 1);

            if (burstUsed == current->burst) {
                current->completed = true;
                current->return_time = time - current->arrive_time + 1;
                completed++;
                current = NULL; 
            }
        }

        time++;
    }

  
    Process* current_process = NULL;
    int next_arrival = 0;

 
    for (int t = 0; t < duration; t++) {
        
        // Afegim els processos que arriben en aquest instant a la cua
        while (next_arrival < nprocs && procTable[next_arrival].arrive_time == t) {
            enqueue(&procTable[next_arrival]);
            next_arrival++;
        }

        // Si no hi ha procés executant-se, treiem un de la cua
        if (current_process == NULL || current_process->completed) {
            current_process = dequeue();
        }

        // Executem el procés actual segons l'algoritme
        if (algorithm == FCFS) {
            if (current_process != NULL) {

                current_process->lifecycle[t] = Running;
                
                int burst_consumed = getCurrentBurst(current_process, t + 1);
                
                if (burst_consumed == 1) {
                    current_process->response_time = t - current_process->arrive_time;
                }
                
                // Si ha acabat el seu burst, marcar com completat
                if (burst_consumed >= current_process->burst) {
                    current_process->lifecycle[t+1] = Finished;
                    current_process->completed = true;
                    current_process->return_time = t + 1 - current_process->arrive_time;
                }
            }
        }

        // Marquem els processos que estan esperant com Bloqued
        for (int p = 0; p < nprocs; p++) {
            if (procTable[p].arrive_time <= t && !procTable[p].completed && 
                &procTable[p] != current_process) {
                procTable[p].lifecycle[t] = Bloqued;
                procTable[p].waiting_time++;
            }
        }

    }

    printSimulation(nprocs,procTable,duration);


    //print metrix

    for (int p=0; p<nprocs; p++ ){
        destroyProcess(procTable[p]);
    }

    cleanQueue();
    return EXIT_SUCCESS;

}


void printSimulation(size_t nprocs, Process *procTable, size_t duration){

    printf("%14s","== SIMULATION ");
    for (int t=0; t<duration; t++ ){
        printf("%5s","=====");
    }
    printf("\n");

    printf ("|%4s", "name");
    for(int t=0; t<duration; t++){
        printf ("|%2d", t);
    }
    printf ("|\n");

    for (int p=0; p<nprocs; p++ ){
        Process current = procTable[p];
            printf ("|%4s", current.name);
            for(int t=0; t<duration; t++){
                printf("|%2s",  (current.lifecycle[t]==Running ? "E" : 
                        current.lifecycle[t]==Bloqued ? "B" :   
                        current.lifecycle[t]==Finished ? "F" : " "));
            }
            printf ("|\n");
        
    }


}

void printMetrics(size_t simulationCPUTime, size_t nprocs, Process *procTable ){

    printf("%-14s","== METRICS ");
    for (int t=0; t<simulationCPUTime+1; t++ ){
        printf("%5s","=====");
    }
    printf("\n");

    printf("= Duration: %ld\n", simulationCPUTime );
    printf("= Processes: %ld\n", nprocs );

    size_t baselineCPUTime = getTotalCPU(procTable, nprocs);
    double throughput = (double) nprocs / (double) simulationCPUTime;
    double cpu_usage = (double) simulationCPUTime / (double) baselineCPUTime;

    printf("= CPU (Usage): %lf\n", cpu_usage*100 );
    printf("= Throughput: %lf\n", throughput*100 );

    double averageWaitingTime = 0;
    double averageResponseTime = 0;
    double averageReturnTime = 0;
    double averageReturnTimeN = 0;

    for (int p=0; p<nprocs; p++ ){
            averageWaitingTime += procTable[p].waiting_time;
            averageResponseTime += procTable[p].response_time;
            averageReturnTime += procTable[p].return_time;
            averageReturnTimeN += procTable[p].return_time / (double) procTable[p].burst;
    }


    printf("= averageWaitingTime: %lf\n", (averageWaitingTime/(double) nprocs) );
    printf("= averageResponseTime: %lf\n", (averageResponseTime/(double) nprocs) );
    printf("= averageReturnTimeN: %lf\n", (averageReturnTimeN/(double) nprocs) );
    printf("= averageReturnTime: %lf\n", (averageReturnTime/(double) nprocs) );

}