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

// Round Robin with nullptr
TEST(RRTest, NullQueue) {
    ScheduleResult_t result;
    bool success = round_robin(NULL, &result, 4);
    EXPECT_FALSE(success);
}

// Round Robin with zero quantum
TEST(RRTest, ZeroQuantum) {
    dyn_array_t *test_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;
    bool success = round_robin(test_queue, &result, 0); 
    EXPECT_FALSE(success); 
    dyn_array_destroy(test_queue);
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

// main: runs all the tests
int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	// ::testing::AddGlobalTestEnvironment(new GradeEnvironment);
	return RUN_ALL_TESTS();
}
