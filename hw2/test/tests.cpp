#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include "gtest/gtest.h"
#include "../include/processing_scheduling.h"
#include "../include/dyn_array.h"

// Using a C library requires extern "C" to prevent function mangling
extern "C"
{
#include <dyn_array.h>
}

#define NUM_PCB 30
#define QUANTUM 5 // Used for Robin Round for process as the run time limit

/*
unsigned int score;
unsigned int total;

class GradeEnvironment : public testing::Environment
{
	public:
		virtual void SetUp()
		{
			score = 0;
			total = 210;
		}

		virtual void TearDown()
		{
			::testing::Test::RecordProperty("points_given", score);
			::testing::Test::RecordProperty("points_total", total);
			std::cout << "SCORE: " << score << '/' << total << std::endl;
		}
};
*/
// Basic corner test for loading PCB (NULL filename)
TEST(LoadPCB, NullFile) {
    dyn_array_t *pcb_array = load_process_control_blocks(NULL);
    EXPECT_EQ(nullptr, pcb_array);
}

// Basic test for loading PCB (BasicFile)
TEST(LoadPCB, BasicFile) {
    // Load from a file
    dyn_array_t *pcb_array = load_process_control_blocks("pcb.bin");
    // If pcb.bin isn't there, this might fail. Make sure your bin is present.
    EXPECT_NE(nullptr, pcb_array);
    if (pcb_array) {
        dyn_array_destroy(pcb_array);
    }
}

// FCFS test with nullptr queue
TEST(FCFSTest, NullQueue) {
    ScheduleResult_t result;
    bool success = first_come_first_serve(NULL, &result);
    EXPECT_FALSE(success);
}

// FCFS test with empty queue
TEST(FCFSTest, EmptyQueue) {
    dyn_array_t *empty_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;
    bool success = first_come_first_serve(empty_queue, &result);
    EXPECT_FALSE(success); // or true if you implement differently
    dyn_array_destroy(empty_queue);
}

// FCFS test with single element in queue
TEST(FCFSTest, SingleQueue) {
    dyn_array_t *single_queue = dyn_array_create(1, sizeof(ProcessControlBlock_t), NULL);
    ProcessControlBlock_t pcb = {
        .remaining_burst_time = 5,
		.priority = 1,				
		.arrival = 0,				
		.started = 0
    };
    dyn_array_push_back(single_queue, &pcb);
    ScheduleResult_t result;
    bool success = first_come_first_serve(single_queue, &result);
    EXPECT_TRUE(success);
    EXPECT_FLOAT_EQ(0.0f, result.average_waiting_time);
    EXPECT_FLOAT_EQ(5.0f, result.average_turnaround_time);
    EXPECT_EQ((unsigned long)5, result.total_run_time);
    dyn_array_destroy(single_queue);
}

// FCFS test with process arriving at different times
TEST(FCFSTest, Queue) {
    dyn_array_t *queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 10, .priority = 1,	.arrival = 0, .started = 0};
    ProcessControlBlock_t pcb2 = { .remaining_burst_time = 2, .priority = 1,	.arrival = 2, .started = 0};
    ProcessControlBlock_t pcb3 = { .remaining_burst_time = 1, .priority = 1,	.arrival = 3, .started = 0};
    dyn_array_push_back(queue, &pcb1);
    dyn_array_push_back(queue, &pcb2);
    dyn_array_push_back(queue, &pcb3);
    ScheduleResult_t result;
    bool success = first_come_first_serve(queue, &result);
    EXPECT_TRUE(success);
    EXPECT_NEAR(result.average_waiting_time, 5.67, 0.01);
    EXPECT_NEAR(result.average_turnaround_time, 10 , 0.01);
    EXPECT_EQ((unsigned long)13, result.total_run_time);
    dyn_array_destroy(queue);
}

// FCFS testing functionality with 6 processes
TEST(FCFSTest, CQueue) {
    dyn_array_t *queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 10, .priority = 1,   .arrival = 6, .started = 0};
    ProcessControlBlock_t pcb2 = { .remaining_burst_time = 2, .priority = 1,    .arrival = 8, .started = 0};
    ProcessControlBlock_t pcb3 = { .remaining_burst_time = 16, .priority = 1,   .arrival = 0, .started = 0};
    ProcessControlBlock_t pcb4 = { .remaining_burst_time = 3, .priority = 1,    .arrival = 25, .started = 0};
    ProcessControlBlock_t pcb5 = { .remaining_burst_time = 50, .priority = 1,   .arrival = 0, .started = 0};
    ProcessControlBlock_t pcb6 = { .remaining_burst_time = 20, .priority = 1,   .arrival = 10, .started = 0};
    dyn_array_push_back(queue, &pcb1);
    dyn_array_push_back(queue, &pcb2);
    dyn_array_push_back(queue, &pcb3);
    dyn_array_push_back(queue, &pcb4);
    dyn_array_push_back(queue, &pcb5);
    dyn_array_push_back(queue, &pcb6);
    ScheduleResult_t result;
    bool success = first_come_first_serve(queue, &result);
    EXPECT_TRUE(success);
    EXPECT_NEAR(result.average_waiting_time, 47.5, 0.01);
    EXPECT_NEAR(result.average_turnaround_time, 64.33 , 0.01);
    EXPECT_EQ(result.total_run_time, 101u);
    dyn_array_destroy(queue);
}

// Shortest Job First with nullptr
TEST(SJFTest, NullQueue) {
    ScheduleResult_t result;
    bool success = shortest_job_first(NULL, &result);
    EXPECT_FALSE(success);
}

// Shortest Job First with empty queue
TEST(SJFTest, EmptyQueue) {
    dyn_array_t *empty_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;
    bool success = shortest_job_first(empty_queue, &result);
    EXPECT_FALSE(success);
    dyn_array_destroy(empty_queue);
}

// FCFS test with single element in queue
TEST(SJFTest, SingleQueue) {
    dyn_array_t *single_queue = dyn_array_create(1, sizeof(ProcessControlBlock_t), NULL);
    ProcessControlBlock_t pcb = {
        .remaining_burst_time = 5,
		.priority = 1,				
		.arrival = 0,				
		.started = 0
    };
    dyn_array_push_back(single_queue, &pcb);
    ScheduleResult_t result;
    bool success = shortest_job_first(single_queue, &result);
    EXPECT_TRUE(success);
    EXPECT_FLOAT_EQ(0.0f, result.average_waiting_time);
    EXPECT_FLOAT_EQ(5.0f, result.average_turnaround_time);
    EXPECT_EQ((unsigned long)5, result.total_run_time);
    dyn_array_destroy(single_queue);
}

// FCFS test with process arriving at different times
TEST(SJFTest, AQueue) {
    dyn_array_t *queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 10, .priority = 1,	.arrival = 0, .started = 0};
    ProcessControlBlock_t pcb2 = { .remaining_burst_time = 2, .priority = 1,	.arrival = 2, .started = 0};
    ProcessControlBlock_t pcb3 = { .remaining_burst_time = 1, .priority = 1,	.arrival = 3, .started = 0};
    dyn_array_push_back(queue, &pcb1);
    dyn_array_push_back(queue, &pcb2);
    dyn_array_push_back(queue, &pcb3);
    ScheduleResult_t result;
    bool success = shortest_job_first(queue, &result);
    EXPECT_TRUE(success);
    EXPECT_NEAR(result.average_waiting_time, 5.33, 0.01);
    EXPECT_NEAR(result.average_turnaround_time, 9.67 , 0.01);
    EXPECT_EQ((unsigned long)13, result.total_run_time);
    dyn_array_destroy(queue);
}

//similat test too A queue
TEST(SJFTest, BQueue) {
    dyn_array_t *queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 5, .priority = 1,	.arrival = 0, .started = 0};
    ProcessControlBlock_t pcb2 = { .remaining_burst_time = 2, .priority = 1,	.arrival = 2, .started = 0};
    ProcessControlBlock_t pcb3 = { .remaining_burst_time = 1, .priority = 1,	.arrival = 4, .started = 0};
    dyn_array_push_back(queue, &pcb1);
    dyn_array_push_back(queue, &pcb2);
    dyn_array_push_back(queue, &pcb3);
    ScheduleResult_t result;
    bool success = shortest_job_first(queue, &result);
    EXPECT_TRUE(success);
    EXPECT_NEAR(result.average_waiting_time, 1.67, 0.01);
    EXPECT_NEAR(result.average_turnaround_time, 4.33 , 0.01);
    EXPECT_EQ((unsigned long)8, result.total_run_time);
    dyn_array_destroy(queue);
}

//difficult six process queue to really test the algorithms
TEST(SJFTest, CQueue) {
    dyn_array_t *queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 10, .priority = 1,	.arrival = 6, .started = 0};
    ProcessControlBlock_t pcb2 = { .remaining_burst_time = 2, .priority = 1,	.arrival = 8, .started = 0};
    ProcessControlBlock_t pcb3 = { .remaining_burst_time = 16, .priority = 1,	.arrival = 0, .started = 0};
    ProcessControlBlock_t pcb4 = { .remaining_burst_time = 3, .priority = 1,	.arrival = 25, .started = 0};
    ProcessControlBlock_t pcb5 = { .remaining_burst_time = 50, .priority = 1,	.arrival = 0, .started = 0};
    ProcessControlBlock_t pcb6 = { .remaining_burst_time = 20, .priority = 1,	.arrival = 10, .started = 0};
    dyn_array_push_back(queue, &pcb1);
    dyn_array_push_back(queue, &pcb2);
    dyn_array_push_back(queue, &pcb3);
    dyn_array_push_back(queue, &pcb4);
    dyn_array_push_back(queue, &pcb5);
    dyn_array_push_back(queue, &pcb6);
    ScheduleResult_t result;
    bool success = shortest_job_first(queue, &result);
    EXPECT_TRUE(success);
    EXPECT_NEAR(result.average_waiting_time, 15.83, 0.01);
    EXPECT_NEAR(result.average_turnaround_time, 32.67 , 0.01);
    EXPECT_EQ((unsigned long)101, result.total_run_time);
    dyn_array_destroy(queue);
}

// Priority with NULL queue
TEST(PriorityTest, NullQueue) {
    ScheduleResult_t result;
    bool success = priority(NULL, &result);
    EXPECT_FALSE(success);
}

// Priority with empty queue
TEST(PriorityTest, EmptyQueue) {
    dyn_array_t *empty_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;
    bool success = priority(empty_queue, &result);
    EXPECT_FALSE(success);
    dyn_array_destroy(empty_queue);
}

// Priority with two processes
TEST(PriorityTest, TwoProcesses) {
    dyn_array_t *queue = dyn_array_create(2, sizeof(ProcessControlBlock_t), NULL);
    
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 5, .priority = 2, .arrival = 0, .started = false };
    ProcessControlBlock_t pcb2 = { .remaining_burst_time = 3, .priority = 1, .arrival = 2, .started = false };
    
    dyn_array_push_back(queue, &pcb1);
    dyn_array_push_back(queue, &pcb2);
    
    ScheduleResult_t result;
    bool success = priority(queue, &result);
    EXPECT_TRUE(success);
    EXPECT_EQ(result.total_run_time, 8u); // 5 + 3 = 8
    EXPECT_NEAR(result.average_waiting_time, 1.5f, 0.01f); // (0 + 3)/2 = 1.5
    EXPECT_NEAR(result.average_turnaround_time, 5.5f, 0.01f); // (5 + 6)/2 = 5.5.
    
    dyn_array_destroy(queue);
}

// Round Robin testing functionality with 6 processes
TEST(PriorityTest, CQueue) {
    dyn_array_t *queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 10, .priority = 1,   .arrival = 6, .started = 0};
    ProcessControlBlock_t pcb2 = { .remaining_burst_time = 2, .priority = 1,    .arrival = 8, .started = 0};
    ProcessControlBlock_t pcb3 = { .remaining_burst_time = 16, .priority = 1,   .arrival = 0, .started = 0};
    ProcessControlBlock_t pcb4 = { .remaining_burst_time = 3, .priority = 1,    .arrival = 25, .started = 0};
    ProcessControlBlock_t pcb5 = { .remaining_burst_time = 50, .priority = 1,   .arrival = 0, .started = 0};
    ProcessControlBlock_t pcb6 = { .remaining_burst_time = 20, .priority = 1,   .arrival = 10, .started = 0};
    dyn_array_push_back(queue, &pcb1);
    dyn_array_push_back(queue, &pcb2);
    dyn_array_push_back(queue, &pcb3);
    dyn_array_push_back(queue, &pcb4);
    dyn_array_push_back(queue, &pcb5);
    dyn_array_push_back(queue, &pcb6);
    ScheduleResult_t result;
    bool success = priority(queue, &result);
    EXPECT_TRUE(success);
    EXPECT_NEAR(result.average_waiting_time, 22.17, 0.01);
    EXPECT_NEAR(result.average_turnaround_time, 39.00 , 0.01);
    EXPECT_EQ(result.total_run_time, 101u);
    dyn_array_destroy(queue);
}

// Round Robin with nullptr
TEST(RRTest, NullQueue) {
    ScheduleResult_t result;
    bool success = round_robin(NULL, &result, 4);
    EXPECT_FALSE(success);
}

// Round Robin with empty queue
TEST(RRTest, EmptyQueue) {
    dyn_array_t *empty_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;
    bool success = round_robin(empty_queue, &result, 2);
    EXPECT_FALSE(success);
    dyn_array_destroy(empty_queue);
}

// Round Robin with zero quantum
TEST(RRTest, ZeroQuantum) {
    dyn_array_t *test_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;
    bool success = round_robin(test_queue, &result, 0); 
    EXPECT_FALSE(success); 
    dyn_array_destroy(test_queue);
}

// Round Robin with two processes
TEST(RRTest, TwoProcesses) {
    dyn_array_t *queue = dyn_array_create(2, sizeof(ProcessControlBlock_t), NULL);
    
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 5, .priority = 1, .arrival = 0, .started = false };
    ProcessControlBlock_t pcb2 = { .remaining_burst_time = 3, .priority = 1, .arrival = 0, .started = false };
    
    dyn_array_push_back(queue, &pcb1);
    dyn_array_push_back(queue, &pcb2);
    
    ScheduleResult_t result;
    bool success = round_robin(queue, &result, 2); // Quantum of 2
    EXPECT_TRUE(success);
    EXPECT_EQ(result.total_run_time, 8u); // 5 + 3 = 8
    EXPECT_NEAR(result.average_waiting_time, 3.5f, 0.01f); // (3 + 4) / 2 = 3.5
    EXPECT_NEAR(result.average_turnaround_time, 7.5f, 0.01f); // (8 + 7) / 2 = 7.5
    
    dyn_array_destroy(queue);
}

// Round Robin testing functionality with 6 processes
TEST(RRTest, CQueue) {
    dyn_array_t *queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 10, .priority = 1,   .arrival = 6, .started = 0};
    ProcessControlBlock_t pcb2 = { .remaining_burst_time = 2, .priority = 1,    .arrival = 8, .started = 0};
    ProcessControlBlock_t pcb3 = { .remaining_burst_time = 16, .priority = 1,   .arrival = 0, .started = 0};
    ProcessControlBlock_t pcb4 = { .remaining_burst_time = 3, .priority = 1,    .arrival = 25, .started = 0};
    ProcessControlBlock_t pcb5 = { .remaining_burst_time = 50, .priority = 1,   .arrival = 0, .started = 0};
    ProcessControlBlock_t pcb6 = { .remaining_burst_time = 20, .priority = 1,   .arrival = 10, .started = 0};
    dyn_array_push_back(queue, &pcb1);
    dyn_array_push_back(queue, &pcb2);
    dyn_array_push_back(queue, &pcb3);
    dyn_array_push_back(queue, &pcb4);
    dyn_array_push_back(queue, &pcb5);
    dyn_array_push_back(queue, &pcb6);
    ScheduleResult_t result;
    bool success = round_robin(queue, &result, 6); // quantum of 6
    EXPECT_TRUE(success);
    EXPECT_NEAR(result.average_waiting_time, 32.17, 0.01);
    EXPECT_NEAR(result.average_turnaround_time, 49.00 , 0.01);
    EXPECT_EQ(result.total_run_time, 101u);
    dyn_array_destroy(queue);
}

// Shortest Remaining Time First with nullptr
TEST(SRTFTest, NullQueue) {
    ScheduleResult_t result;
    bool success = shortest_remaining_time_first(NULL, &result);
    EXPECT_FALSE(success);
}

// Shortest Remaining Time First with empty queue
TEST(SRTFTest, EmptyQueue) {
    dyn_array_t *empty_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;
    bool success = shortest_remaining_time_first(empty_queue, &result);
    EXPECT_FALSE(success);
    dyn_array_destroy(empty_queue);
}

// Shortest Remaining Time First with a single process
TEST(SRTFTest, SingleProcess) {
    //New queue
    dyn_array_t *queue = dyn_array_create(1, sizeof(ProcessControlBlock_t), NULL);

    //Single process
    ProcessControlBlock_t pcb = { .remaining_burst_time = 5, .priority = 1, .arrival = 0, .started = false };

    //Pushing to the back of the queue
    dyn_array_push_back(queue, &pcb);

    ScheduleResult_t result;
    bool success = shortest_remaining_time_first(queue, &result);
    EXPECT_TRUE(success);
    
    EXPECT_EQ(result.total_run_time, 5u);
    EXPECT_FLOAT_EQ(result.average_waiting_time, 0.0f);
    EXPECT_FLOAT_EQ(result.average_turnaround_time, 5.0f);
    dyn_array_destroy(queue);
}

// Shortest Remaining Time First with 3 processes.
TEST(SRTFTest, ValidQueue) {
    // Creating new queue
    dyn_array_t *queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);

    // Inline Data
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 6, .priority = 1, .arrival = 0, .started = false };
    ProcessControlBlock_t pcb2 = { .remaining_burst_time = 4, .priority = 1, .arrival = 2, .started = false };
    ProcessControlBlock_t pcb3 = { .remaining_burst_time = 2, .priority = 1, .arrival = 4, .started = false };

    //Pushing to the back of the array
    dyn_array_push_back(queue, &pcb1);
    dyn_array_push_back(queue, &pcb2);
    dyn_array_push_back(queue, &pcb3);

    ScheduleResult_t result;
    bool success = shortest_remaining_time_first(queue, &result);
    EXPECT_TRUE(success);

    EXPECT_NEAR(result.average_waiting_time, 2.67, 0.01);
    EXPECT_NEAR(result.average_turnaround_time, 6.67, 0.01);
    EXPECT_EQ(result.total_run_time, 12u);

    dyn_array_destroy(queue);
}

// SRTF testing functionality with 6 processes
TEST(SRTFTest, CQueue) {
    dyn_array_t *queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 10, .priority = 1,   .arrival = 6, .started = 0};
    ProcessControlBlock_t pcb2 = { .remaining_burst_time = 2, .priority = 1,    .arrival = 8, .started = 0};
    ProcessControlBlock_t pcb3 = { .remaining_burst_time = 16, .priority = 1,   .arrival = 0, .started = 0};
    ProcessControlBlock_t pcb4 = { .remaining_burst_time = 3, .priority = 1,    .arrival = 25, .started = 0};
    ProcessControlBlock_t pcb5 = { .remaining_burst_time = 50, .priority = 1,   .arrival = 0, .started = 0};
    ProcessControlBlock_t pcb6 = { .remaining_burst_time = 20, .priority = 1,   .arrival = 10, .started = 0};
    dyn_array_push_back(queue, &pcb1);
    dyn_array_push_back(queue, &pcb2);
    dyn_array_push_back(queue, &pcb3);
    dyn_array_push_back(queue, &pcb4);
    dyn_array_push_back(queue, &pcb5);
    dyn_array_push_back(queue, &pcb6);
    ScheduleResult_t result;
    bool success = shortest_remaining_time_first(queue, &result);
    EXPECT_TRUE(success);
    EXPECT_NEAR(result.average_waiting_time, 14.83, 0.01);
    EXPECT_NEAR(result.average_turnaround_time, 31.67 , 0.01);
    EXPECT_EQ(result.total_run_time, 101u);
    dyn_array_destroy(queue);
}

// main: runs all the tests
int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	// ::testing::AddGlobalTestEnvironment(new GradeEnvironment);
	return RUN_ALL_TESTS();
}
