// Filename: BeatDetectionDriver.cpp
// EECS452 Beat Detection Group
// This is the main function for the architecture
// Initializes the hardware and FIFOS, and then starts
// the worker threads

#include <condition_variable>
#include "InputDataThread.h"

std::deque<int16_t> SAMPLE_QUEUE;
std::deque<double> RMS_AMPLITUDE;

int main () 
{	
	SAMPLE_QUEUE.resize(SAMPLE_QUEUE_SIZE, 0);  // SAMPLE_QUEUE will store B*H = 1920 samples at a time
	RMS_AMPLITUDE.resize(100, 0);	// RMS_AMPLITUDE will store 100 downsampled amplitude values at a time
	SLOPES.resize(97, 0)	// SLOPES can only contain 97 values at a time if amplitude contains 100 values
	std::cout << "Welcome to Beat Detection! Press a key to continue\n";
	std::string tmp;
	std::cin >> tmp;

	// Start the driving function 
	collect_and_do_onsets();
}
