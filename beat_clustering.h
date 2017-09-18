#ifndef BEAT_CLUSTERING_H_
#define BEAT_CLUSTERING_H_

#include "Objects.h"

void clustering_real_time(std::deque<Onset> &);

void do_clustering(Onset *, Onset *);

void merge_clusters();

void score_clusters();

void add_new_onsets(std::deque<Onset> &);

#endif