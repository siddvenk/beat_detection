// File name: InputDataThread.h
//
// 20Oct2016 .. intial version for EECS 452 lab 6 .. KM

#ifndef ONSET_DETECTION_THREAD_H_
#define ONSET_DETECTION_THREAD_H_

#include <iostream>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "globals.h"
#include "beat_tracking.h"
#include "beat_clustering.h"

void onset_detection(std::deque<float> &);

#endif