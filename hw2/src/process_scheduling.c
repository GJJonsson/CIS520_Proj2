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
	if(!ready_queue || !result) return false;
    size_t n = dyn_array_size(ready_queue);
    if(n == 0) return false;

    unsigned long current_time = 0;
    float total_wait = 0.0f, total_turn = 0.0f;
    bool completed_array[n];
    size_t number_completed = 0;

    for(size_t i = 0; i < n; i++){
        completed_array[i] = 0;
    }

    //sort array by arrival time
    if(!dyn_array_sort(ready_queue, pcb_arrival_cmp)) {
        return false;
    }

    //we want to keep looping until all values have been processed
    while(number_completed < n){

        //smallest burst value in data set
        size_t smallest_burst = SIZE_MAX;
        //the index of smallest burst within system time
        size_t optimal_choice = SIZE_MAX;

        //loop to find optimal choice index
        for(size_t j = 0; j < n; j++){

            //skip if a completed value
            if(completed_array[j] == 1){
                continue;
            }

            ProcessControlBlock_t *pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, j);
            if(!pcb) return false;

            //if true a possible optimal choice found
            if(pcb->arrival <= current_time && pcb->remaining_burst_time < smallest_burst){
                smallest_burst = pcb->remaining_burst_time;
                optimal_choice = j;
            }
        }

        //if no  optimal choice found we update system time
        if(optimal_choice == SIZE_MAX){
            for(size_t j = 0; j < n; j++){
                
                //skip completed values 
                if(completed_array[j] == 1){
                    continue;
                }
                //update current time to smallest arrival time
                else{
                    ProcessControlBlock_t *pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, j);
                    if(!pcb) return false;
                    current_time = pcb->arrival;
                    break;
                }
            }
        }
        //else optimal choice found
        else{

            //mark index in array as completed
            completed_array[optimal_choice] = 1;
            //update number completed
            number_completed++;

            ProcessControlBlock_t *pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, optimal_choice);
            if(!pcb) return false;
            
            // waiting time is when we actually start - arrival
            float wait = (float)(current_time - pcb->arrival);
            total_wait += wait;

            // run the process fully
            current_time += pcb->remaining_burst_time;

            // turnaround = completion - arrival
            float turn = (float)(current_time - pcb->arrival);
            total_turn += turn;
        }
    }

    // fill out result
    result->average_waiting_time = total_wait / (float)n;
    result->average_turnaround_time = total_turn / (float)n;
    result->total_run_time = current_time;

    return true;
}

bool priority(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
     //Checking input pointers
    if (!ready_queue || !result)
    {
        return false;
    }

    // Checking for empty queue
    if (dyn_array_empty(ready_queue))
    {
        return false;
    }
        
    size_t n = dyn_array_size(ready_queue);
    
    // Allocate temporary arrays to track scheduling statistics.
    bool *done = (bool *)malloc(n * sizeof(bool));
    uint32_t *burst = (uint32_t *)malloc(n * sizeof(uint32_t));
    uint32_t *waiting = (uint32_t *)malloc(n * sizeof(uint32_t));
    uint32_t *turnaround = (uint32_t *)malloc(n * sizeof(uint32_t));

    // Initialize arrays.
    for (size_t i = 0; i < n; i++) 
    {
        done[i] = false;
        waiting[i] = 0;
        turnaround[i] = 0;
        ProcessControlBlock_t *pcb = (ProcessControlBlock_t *)dyn_array_at(ready_queue, i);
        burst[i] = pcb->remaining_burst_time; // Original burst time.
    }
    
    uint32_t time = 0;
    size_t completed = 0;
    
    // Simulate the scheduling until all processes are completed.
    while (completed < n) 
    {
        int index = 0;
        uint32_t best_priority = UINT32_MAX;
        
        // Find the not-yet-done process that has arrived and has the highest priority (lowest number).
        for (size_t i = 0; i < n; i++) 
        {
            ProcessControlBlock_t *pcb = (ProcessControlBlock_t *)dyn_array_at(ready_queue, i);
            if (!done[i] && pcb->arrival <= time) //If arrived (The addition of !done[i] was suggested by ChatGPT while debugging)
            {
                if (pcb->priority < best_priority) // If a process with lower priority is found
                {
                    best_priority = pcb->priority; //updating priority
                    index = (int)i; // Updating index
                }
            }
        }
        
        if (index == -1) {
            time++;
            continue;
        }

        // Process idx is scheduled.
        ProcessControlBlock_t *pcb = (ProcessControlBlock_t *)dyn_array_at(ready_queue, index);
        waiting[index] = time - pcb->arrival;
        time += burst[index];  // Run the process to completion.
        turnaround[index] = time - pcb->arrival;
        done[index] = true;
        completed++;
    }
    
    uint32_t total_waiting = 0;
    uint32_t total_turnaround = 0;
    for (size_t i = 0; i < n; i++) 
    {
        total_waiting += waiting[i];
        total_turnaround += turnaround[i];
    }

    // Assigning values
    result->total_run_time = time;
    result->average_waiting_time = (float)total_waiting / n;
    result->average_turnaround_time = (float)total_turnaround / n;
    
    free(done);
    free(burst);
    free(waiting);
    free(turnaround);
    return true;
}

bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
     // Checking for invalid pointers
    if (!ready_queue || !result)
    {
        return false;
    }  
        
    // Checking for empty array
    if (dyn_array_empty(ready_queue))
    {
        return false;
    }

    // Checking for 0 quantum
    if (quantum == 0)
    {
        return false;
    }

    size_t n = dyn_array_size(ready_queue);
    
    // Allocating arrays 
    uint32_t *orig = (uint32_t *)malloc(n * sizeof(uint32_t));
    uint32_t *remaining = (uint32_t *)malloc(n * sizeof(uint32_t));
    uint32_t *finish = (uint32_t *)malloc(n * sizeof(uint32_t));
    uint32_t *arrival = (uint32_t *)malloc(n * sizeof(uint32_t));
    
    // Initializing arrays
    for (size_t i = 0; i < n; i++)
    {
        ProcessControlBlock_t *pcb = (ProcessControlBlock_t *)dyn_array_at(ready_queue, i);
        orig[i] = pcb->remaining_burst_time;
        remaining[i] = pcb->remaining_burst_time;
        arrival[i] = pcb->arrival;
        finish[i] = 0;
    }
    
    uint32_t time = 0;
    size_t completed = 0;
    
    // Simulating the Round Robin scheduling.
    while (completed < n) 
    {
        bool ran_proc = false; /* track if anything ran this pass */
        for (size_t i = 0; i < n; i++) 
        {
            
            if (arrival[i] <= time && remaining[i] > 0) // IF processes that have arrived and are not finished.
            {
                ran_proc = true;
                uint32_t timeSlice;
                if (remaining[i] < quantum) 
                {
                    timeSlice = remaining[i]; // Whathever is remaining
                } 
                else 
                {
                    timeSlice = quantum; // The full quantum
                }

                time += timeSlice; // Adding to the current time
                remaining[i] -= timeSlice; // Removing from remaining
                
                if (remaining[i] == 0) // If the process finishes
                {
                    finish[i] = time;// Saving its finish time.
                    completed++;
                }
            }
        }
        /* If no process ran in that entire for-loop, CPU was idle; move time forward by 1 */
        if(!ran_proc) {
            time++;
        }
    }
    
    uint32_t total_waiting = 0;
    uint32_t total_turnaround = 0;
    for (size_t i = 0; i < n; i++) 
    {
        // Calculating total waiting time and turnaround time.
        uint32_t turnaround = finish[i] - arrival[i];
        uint32_t waiting = turnaround - orig[i];
        total_turnaround += turnaround;
        total_waiting += waiting;
    }
    
    // Assigning values
    result->total_run_time = time;
    result->average_waiting_time = (float)total_waiting / n;
    result->average_turnaround_time = (float)total_turnaround / n;
    
    free(orig);
    free(remaining);
    free(finish);
    free(arrival);
    
    return true;
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
    //Checking for invalid pointers
    if (!ready_queue || !result) 
    {
        return false;
    }

    size_t n = dyn_array_size(ready_queue);
    if (n == 0) // If empty
    {
        return false;
    }

    // Making a copy of the PCBs so we don't modify the original ready_queue.
    ProcessControlBlock_t *processes = malloc(n * sizeof(ProcessControlBlock_t));
    if (!processes) 
    {
        return false;
    }

    // Iterating to copy data to processes
    for (size_t i = 0; i < n; i++) 
    {
        ProcessControlBlock_t *pcb = (ProcessControlBlock_t *)dyn_array_at(ready_queue, i);
        if (!pcb) // If not valid
        {
            free(processes);
            return false;
        }
        // if pointer is valid
        processes[i] = *pcb; // Copy data
    }
    
    uint32_t *start_times = malloc(n * sizeof(uint32_t)); // Records when a process is first executed.
    uint32_t *finish_times = malloc(n * sizeof(uint32_t));// Records when a process completes.
    uint32_t *original_burst = malloc(n * sizeof(uint32_t));// Stores the initial burst time.

    for (size_t i = 0; i < n; i++) 
    {
        start_times[i] = UINT32_MAX;  // flag value to indicate not started
        finish_times[i] = 0;
        original_burst[i] = processes[i].remaining_burst_time;
    }

    uint32_t time = 0;
    size_t completed = 0;
    while (completed < n) // While there are still processes to be completed
    {
        int index = 0;
        uint32_t min_remaining = UINT32_MAX;
 
        for (size_t i = 0; i < n; i++) 
        {
            if (processes[i].remaining_burst_time > 0 && processes[i].arrival <= time) // Finding the process that has arrived
            {
                if (processes[i].remaining_burst_time < min_remaining) // Finding procees with the smallest remaining burst time.
                {
                    min_remaining = processes[i].remaining_burst_time;
                    index = (int)i; //update index
                }
            }
        }
        if(index<0){
            /* no arrived process found, so CPU is idle */
            time++;
            continue;
        }
        // Executing the selected process
        processes[index].remaining_burst_time--;
        time++;
        
        if (processes[index].remaining_burst_time == 0)  // If the process has finished executing
        {
            finish_times[index] = time; //Save its finish time.
            completed++; // Update completed counter
        }
    }

    uint32_t total_waiting_time = 0;
    uint32_t total_turnaround_time = 0;

    for (size_t i = 0; i < n; i++)
    {
        // Computing total waiting time and turnaround time.
        uint32_t turnaround = finish_times[i] - processes[i].arrival;
        uint32_t waiting = turnaround - original_burst[i];
        total_turnaround_time += turnaround;
        total_waiting_time += waiting;
    }

    //Assinging average values
    result->average_waiting_time = (float)total_waiting_time / n;
    result->average_turnaround_time = (float)total_turnaround_time / n;
    result->total_run_time = time;

    free(processes);
    free(start_times);
    free(finish_times);
    free(original_burst);
    return true;
}

static int pcb_arrival_cmp(const void *a, const void *b) {
    const ProcessControlBlock_t *pa = (const ProcessControlBlock_t*)a;
    const ProcessControlBlock_t *pb = (const ProcessControlBlock_t*)b;
    if(pa->arrival < pb->arrival) return -1;
    else if(pa->arrival > pb->arrival) return 1;
    return 0;
}
