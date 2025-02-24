#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dyn_array.h"
#include "processing_scheduling.h"


// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

static int pcb_arrival_cmp(const void *a, const void *b);


// private function
void virtual_cpu(ProcessControlBlock_t *process_control_block) 
{
	// decrement the burst time of the pcb
	--process_control_block->remaining_burst_time;
}

// Implements a queue for the processes coming in.
bool first_come_first_serve(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    if(!ready_queue || !result) return false;
    size_t n = dyn_array_size(ready_queue);
    if(n == 0) return false;

    // sort by arrival
    if(!dyn_array_sort(ready_queue, pcb_arrival_cmp)) {
        return false;
    }

    unsigned long current_time = 0;
    float total_wait = 0.0f, total_turn = 0.0f;

    for(size_t i=0; i<n; i++) {
        ProcessControlBlock_t *pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, i);
        if(!pcb) return false;

        // if process arrives later than current_time, jump time forward
        if(current_time < pcb->arrival) {
            current_time = pcb->arrival;
        }
        // waiting time is when we actually start - arrival
        float wait = (float)(current_time - pcb->arrival);
        total_wait += wait;

        // run the process fully
        current_time += pcb->remaining_burst_time;

        // turnaround = completion - arrival
        float turn = (float)(current_time - pcb->arrival);
        total_turn += turn;
    }

    // fill out result
    result->average_waiting_time = total_wait / (float)n;
    result->average_turnaround_time = total_turn / (float)n;
    result->total_run_time = current_time;

    return true;
}

bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}

bool priority(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}

bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	UNUSED(quantum);
	return false;
}

// Loads the process from the PCB File.
dyn_array_t *load_process_control_blocks(const char *input_file) 
{
    if(!input_file) return NULL; // corner case

    FILE *fp = fopen(input_file, "rb");
    if(!fp) return NULL; // file open error

    uint32_t N = 0;
    size_t items_read = fread(&N, sizeof(uint32_t), 1, fp);
    if(items_read != 1) {
        fclose(fp);
        return NULL;
    }

    dyn_array_t *arr = dyn_array_create(N, sizeof(ProcessControlBlock_t), NULL);
    if(!arr) {
        fclose(fp);
        return NULL;
    }

    for(uint32_t i=0; i<N; i++) {
        ProcessControlBlock_t temp;
        size_t r1 = fread(&temp.remaining_burst_time, sizeof(uint32_t), 1, fp);
        size_t r2 = fread(&temp.priority, sizeof(uint32_t), 1, fp);
        size_t r3 = fread(&temp.arrival, sizeof(uint32_t), 1, fp);
        temp.started = false;
        if(r1!=1 || r2!=1 || r3!=1) {
            dyn_array_destroy(arr);
            fclose(fp);
            return NULL;
        }
        if(!dyn_array_push_back(arr, &temp)) {
            dyn_array_destroy(arr);
            fclose(fp);
            return NULL;
        }
    }
    fclose(fp);
    return arr;
}

bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}

static int pcb_arrival_cmp(const void *a, const void *b) {
    const ProcessControlBlock_t *pa = (const ProcessControlBlock_t*)a;
    const ProcessControlBlock_t *pb = (const ProcessControlBlock_t*)b;
    if(pa->arrival < pb->arrival) return -1;
    else if(pa->arrival > pb->arrival) return 1;
    return 0;
}
