// File name: globals.h

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <stdint.h>
#include <deque>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <vector>

#include "Objects.h"

// Some global variables
const long FS = 48000;
const int B = 0.01*FS;
const int H = 4;
const int SAMPLE_QUEUE_SIZE = B * H;
const long long SILENCE_TO_RESET_SAMPLES = 4*FS;
const int SILENCE_THRESHOLD_POSITIVE = -80;
const int SILENCE_THRESHOLD_NEGATIVE = -90;
const int m = 3;
const int l = 3;
const int AMPLITUDE_THRESHOLD = 3000; // Need to set to proper value
const int SLOPE_THRESHOLD = 2; // Need to set to proper value

// External stuff necessary to score
extern std::vector<Cluster > clusters_max;
extern Agent* highest_score_agent;
extern bool agents_start;
extern std::deque<Onset> onsets;
extern std::vector<Cluster *> clusters;
extern float averageIOI;
extern int bestScore;

// Receive samples as 16 bit signed integers
extern std::deque<int16_t> SAMPLE_QUEUE;

// Queue to store the RMS values
extern std::deque<double> RMS_AMPLITUDE;

// Represents slopes at the corresponging RMS_AMPLITUDE indexes
extern std::deque<double> SLOPES;

extern std::deque<Onset> ONSETS;

#endif