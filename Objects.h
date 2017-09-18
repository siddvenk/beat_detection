/*
 * This file describes an Onset, Cluster, and Agent
 * which are used in the clustering and beat tracking
 * systems
 */
#ifndef OBJECTS_H
#define OBJECTS_H

#include <cstdlib>
#include <cmath>
#include <vector>
#include <deque>
#include <list>
#include <iostream>
#include <fstream>
#include <string>

const int MAX_ONSETS_IN_MEMORY = 50;
const int ONSETS_NEEDED_TO_START = 10;
const int fs = 100;
const float max_tempo = fs * 100 / 1000.0;
const float min_tempo = fs * 2000 / 1000.0;
const float cluster_width = fs * 25 / 1000.0;//using same width to different tempo
const int SCORE[] = { 5,4,3,2,1,1,1,1 };
const int TIME_OUT = fs * 5;
const int CORRECTION_FACTOR = 1000;
const int tolerance_inner = fs * 40 / 1000;

struct Onset {
	long time_stamp;
	double magnitude;
	Onset() : time_stamp(0), magnitude(0) {}
	Onset(long _time_stamp, double _magnitude) : time_stamp(_time_stamp), magnitude(_magnitude) {}
};

struct Cluster {
	std::list<float> inter_onset_intervals;
	//std::list<Onset> onsets;
	float average_IOI;
	float total_IOI;
	int score;
};
 
struct Agent {
	float interval;
	float prediction;
	//std::vector<Onset > history;
	Onset history;
	float score;
	Agent() : interval(0), prediction(0), history(Onset()), score(0) {}
};

// global list of clusters that changes throughout song
extern std::vector<Cluster *> clusters;

// global list of onsets, only a limited amount are kept in memory at a time
extern std::deque<Onset *> onsets;

extern std::vector<Agent *> agents;

#endif