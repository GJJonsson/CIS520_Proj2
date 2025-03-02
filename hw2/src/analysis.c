#include <stdio.h>
#include <stdlib.h>
#define SRT "SRT"

#include "dyn_array.h"
#include "processing_scheduling.h"

#define FCFS "FCFS"
#define P "P"
#define RR "RR"
#define SJF "SJF"

//Github test message

// Add and comment your analysis code in this function.
// THIS IS NOT FINISHED.
int main(int argc, char **argv) 
{
    if(argc < 3) {
        printf("Usage: %s <pcb file> <schedule algorithm>\n", argv[0]);
        return EXIT_FAILURE;
    }

    dyn_array_t *pcbs = load_process_control_blocks(argv[1]);
    if(!pcbs) {
        printf("Error loading PCBs.\n");
        return EXIT_FAILURE;
    }

    ScheduleResult_t res = {0};
    res.average_turnaround_time = 0;
    res.average_waiting_time = 0;
    res.total_run_time = 0;
    if(strncmp(argv[2], "FCFS", 4) == 0) {
        if(!first_come_first_serve(pcbs, &res)) {
            printf("FCFS scheduling failed.\n");
            dyn_array_destroy(pcbs);
            return EXIT_FAILURE;
        }
    }
    else if(strncmp(argv[2], "SJF", 3) == 0) {
        if(!shortest_job_first(pcbs, &res)) {
            printf("SJF failed.\n");
            dyn_array_destroy(pcbs);
            return EXIT_FAILURE;
        }
    }
    else if(strncmp(argv[2], "P", 1) == 0) {
        if(!priority(pcbs, &res)) {
            printf("Priority failed.\n");
            dyn_array_destroy(pcbs);
            return EXIT_FAILURE;
        }
    }
    else if(strncmp(argv[2], "RR", 2) == 0) {
        if(argc < 4) {
            printf("Must supply quantum for RR.\n");
            dyn_array_destroy(pcbs);
            return EXIT_FAILURE;
        }
        int q = 0;
        if(sscanf(argv[3], "%d", &q) != 1 || q <= 0) {
            printf("Bad quantum.\n");
            dyn_array_destroy(pcbs);
            return EXIT_FAILURE;
        }
        if(!round_robin(pcbs, &res, (size_t)q)) {
            printf("RR failed.\n");
            dyn_array_destroy(pcbs);
            return EXIT_FAILURE;
        }
    }
    else if(strncmp(argv[2], "SRT", 3) == 0) {
        if(!shortest_remaining_time_first(pcbs, &res)) {
            printf("SRT failed.\n");
            dyn_array_destroy(pcbs);
            return EXIT_FAILURE;
        }
    }
    else {
        printf("Unknown alg.\n");
        dyn_array_destroy(pcbs);
        return EXIT_FAILURE;
    }


    printf("Avg Wait: %.2f\n", res.average_waiting_time);
    printf("Avg Turnaround: %.2f\n", res.average_turnaround_time);
    printf("Total Time: %lu\n", res.total_run_time);

    dyn_array_destroy(pcbs);
    return EXIT_SUCCESS;
}
