#ifndef BEAT_TRACKING_H_
#define BEAT_TRACKING_H_

#include "Objects.h"

void beat_tracking_initialisation(std::vector<Cluster> &, std::deque<Onset> &, int);

Agent* beat_tracking(std::deque<Onset> &);

Agent* beat_tracking_update(std::deque<Onset> &);

void new_clusters_tracking(std::vector<Cluster *> &, std::vector<Cluster > &, Agent*);

#endif