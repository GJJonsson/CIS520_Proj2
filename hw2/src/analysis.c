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
    if(strncmp(argv[2], "FCFS", 4) == 0) {
        if(!first_come_first_serve(pcbs, &res)) {
            printf("FCFS scheduling failed.\n");
            dyn_array_destroy(pcbs);
            return EXIT_FAILURE;
        }
    }
    // else if(...) other schedulers

    printf("Avg Wait: %.2f\n", res.average_waiting_time);
    printf("Avg Turnaround: %.2f\n", res.average_turnaround_time);
    printf("Total Time: %lu\n", res.total_run_time);

    dyn_array_destroy(pcbs);
    return EXIT_SUCCESS;
}
