// File name: InputDataThread.h
//
// 20Oct2016 .. intial version for EECS 452 lab 6 .. KM

#ifndef INPUT_DATA_THREAD_H_
#define INPUT_DATA_THREAD_H_

#include <stdlib.h>
#include <string>
#include <wiringPiSPI.h>
#include <wiringPi.h>
#include <time.h>
#include <iostream>
#include "globals.h"
#include "FIFO_builder.h"
#include "OnsetDetectionThread.h"

void collect_samples();

void calculate_onsets(std::deque<int16_t> &, int &, long &);

#endif