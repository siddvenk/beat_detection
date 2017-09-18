#include "beat_clustering.h"
// global list of clusters that changes throughout song
std::vector<Cluster *> clusters;

// global list of onsets, only a limited amount are kept in memory at a time
std::deque<Onset> onsets;
float averageIOI = -1;
int bestScore = 0;

void clustering_real_time(std::deque<Onset> &new_onsets)
{
	// See if we have enough onsets to start doing clustering
	// if (onsets.size() < ONSETS_NEEDED_TO_START) {
	// 	for (int i = 0; i < new_onsets.size(); ++i) {
	// 		onsets.push_back(new_onsets[i]);
	// 	}
	// 	return;
	// }

	// Calculate the IOIs between new onsets and old onsets
	for (int i = 0; i < new_onsets.size(); ++i) {
		for (int j = 0; j < onsets.size(); ++j) {
			do_clustering(&new_onsets[i], &onsets[j]);
		}
	}

	// Calculate the IOIS between the new onsets
	for (int i = new_onsets.size() - 1; i > 0; --i) {
		for (int j = i - 1; j >= 0; --j) {
			do_clustering(&new_onsets[i], &new_onsets[j]);
		}
	}

	// Now merge the clusters is necessary
	merge_clusters();

	// Now score the merged clusters
	score_clusters();

	// Add new onsets to the onsets list
	add_new_onsets(new_onsets);
}

void do_clustering(Onset *later, Onset *earlier)
{
	float IOI = later->time_stamp - earlier->time_stamp;
	bool found_cluster = false;
	if (IOI > min_tempo || IOI < max_tempo) return;

	for (int k = 0; k < clusters.size(); ++k) {
		// Find a cluster to add this IOI to
		if (abs(clusters[k]->average_IOI - IOI) < cluster_width) {
			clusters[k]->inter_onset_intervals.push_back(IOI);
			//clusters[k]->onsets.push_back(*earlier);
			clusters[k]->total_IOI += IOI;
			clusters[k]->average_IOI = clusters[k]->total_IOI / clusters[k]->inter_onset_intervals.size();
			found_cluster = true;
			break;
		}

	}

	// if no cluster was found, create a new one
	if (!found_cluster) {
		Cluster *c = new Cluster();
		c->inter_onset_intervals.push_back(IOI);
		//c->onsets.push_back(*earlier);
		c->average_IOI = IOI;
		c->total_IOI = IOI;
		c->score = 0;
		clusters.push_back(c);
	}
}

void merge_clusters()
{
	for (int i = 0; i < clusters.size(); ++i) {
		for (int j = i + 1; j < clusters.size(); ++j) {
			if (i != j) {
				if (abs(clusters[i]->average_IOI - clusters[j]->average_IOI) < cluster_width) {
					clusters[i]->inter_onset_intervals.splice(clusters[i]->inter_onset_intervals.end(), clusters[j]->inter_onset_intervals);
					//clusters[i]->onsets.splice(clusters[i]->onsets.end(), clusters[j]->onsets);
					clusters[i]->total_IOI = clusters[i]->total_IOI + clusters[j]->total_IOI;
					delete clusters[j];
					clusters.erase(clusters.begin() + j);
				}
			}
		}
	}
}

void score_clusters()
{
	for (int i = 0; i < clusters.size(); ++i) {
		clusters[i]->score = 0;
		for (int j = 0; j < clusters.size(); ++j) {
			for (int n = 1; n < 8; ++n) {
				if (abs(clusters[i]->average_IOI - n*clusters[j]->average_IOI) < cluster_width * n) {
					clusters[i]->score += SCORE[n - 1] * clusters[j]->inter_onset_intervals.size();
				}
				if (abs(n*clusters[i]->average_IOI - clusters[j]->average_IOI) < cluster_width / n) {
					clusters[i]->score += SCORE[n - 1] * clusters[j]->inter_onset_intervals.size();
				}
				if (clusters[i]->score > bestScore) {
					bestScore = clusters[i]->score;
					averageIOI = clusters[i]->average_IOI;
				}
			}
		}
	}
}

void add_new_onsets(std::deque<Onset> &new_onsets)
{
	// If memory is full, remove num_onsets, then add new_onsets
	if (onsets.size() == MAX_ONSETS_IN_MEMORY) {
		for (int i = 0; i < new_onsets.size(); ++i) {
			onsets.pop_front();
			onsets.push_back(new_onsets[i]);
		}
		return;
	}

	// Otherwise there is still space to add new onsets without removing old ones
	int i = 0;
	while (onsets.size() < MAX_ONSETS_IN_MEMORY && i < new_onsets.size()) {
		onsets.push_back(new_onsets[i]);
		++i;
	}

	while (i < new_onsets.size()) {
		onsets.pop_front();
		onsets.push_back(new_onsets[i]);
		++i;
	}
}