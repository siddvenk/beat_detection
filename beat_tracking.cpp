/*
 * This file describes a function that takes in 
 * a set of clusters, and uses the clusters to 
 * make a prediction about the beat of the song
 *
 */

#include "beat_tracking.h"

std::vector<Agent *> agents;

// NOTE: THIS FUNCTION SHOULD ONLY BE CALLED ONCE FROM SOMEWHERE ELSE SINCE REAL TIME
void beat_tracking_initialisation(std::vector<Cluster > &clusters, std::deque<Onset> &onsets, int numClusters)
{
	// Create the initial agents based on the first set of onsets
	for (int i = clusters.size() - numClusters; i < clusters.size(); ++i) {
		for (int j = 0; j < 20; ++j) {
			Agent *new_agent = new Agent();
			new_agent->interval = clusters[i].average_IOI;
			new_agent->prediction = onsets[j].time_stamp + new_agent->interval;
			new_agent->history = onsets[j];
			new_agent->score = onsets[j].magnitude;
			agents.push_back(new_agent);
		}
	}
}


Agent* beat_tracking(std::deque<Onset> &new_onsets)
{
	std::vector<Agent *> new_agents;
	static Agent *highest_score_agent = agents[0];
	for (int i = 0; i < new_onsets.size(); ++i) {
		for (int j = 0; j < agents.size(); ++j) {
			// If this new onset is way past agent's last onset, this agent is invalid
			if (new_onsets[i].time_stamp - agents[j]->history.time_stamp > TIME_OUT) {
				agents.erase(agents.begin() + j);
			}
			// Otherwise create tolerance windows and check whether this onset can be added to this agent
			else {
				double tolerance_pre = 0.2 * agents[j]->interval;   // 20% of current interval
				double tolerance_post = 0.4 * agents[j]->interval;  // 40% of current interval

				while (agents[j]->prediction + tolerance_post < new_onsets[i].time_stamp) {
					agents[j]->prediction += agents[j]->interval;
				}

				// This onset falls within an existing agent's prediction interval
				if ((new_onsets[i].time_stamp >= agents[j]->prediction - tolerance_pre) &&
					(new_onsets[i].time_stamp < agents[j]->prediction + tolerance_post)) {
						// Now see if onset falls between inner and outer interval
						if (std::abs(agents[j]->prediction - new_onsets[i].time_stamp) > tolerance_inner) {
							Agent *new_agent = new Agent();
							new_agent->interval = agents[j]->interval;
							new_agent->prediction = agents[j]->prediction;
							new_agent->history = agents[j]->history;
							new_agent->score = agents[j]->score;
							new_agents.push_back(new_agent);
						}

						double error = new_onsets[i].time_stamp - agents[j]->prediction;
						double relative_error;
						if (error <= 0) relative_error = -error / tolerance_pre;
						else relative_error = error / tolerance_post;
						
						agents[j]->interval += error / CORRECTION_FACTOR;
						agents[j]->prediction = new_onsets[i].time_stamp + agents[j]->interval;
						agents[j]->history = new_onsets[i];
						agents[j]->score += (1 - relative_error/2) * new_onsets[i].magnitude;
						if (agents[j]->score > highest_score_agent->score) {
							highest_score_agent = agents[j];
						}
				}
			}
		}
	}

	// Add new agents
	for (int i = 0; i < new_agents.size(); ++i) {
		agents.push_back(new_agents[i]);
	}

	// Remove duplicate agents
	for (int i = 0; i < agents.size(); ++i) {
		for (int j = i+1; j < agents.size(); ++j) {
			if ((std::abs(agents[i]->interval - agents[j]->interval) < fs * 20/1000) &&
				(std::abs(agents[i]->prediction - agents[j]->prediction) < fs * 40/1000)) {
				if (agents[i]->score >= agents[j]->score) {
					delete agents[j];
					agents.erase(agents.begin() + j);
				}
				else {
					delete agents[i];
					agents.erase(agents.begin() + i);
				}
				}
		}
	}

	return highest_score_agent;
}

Agent* beat_tracking_update(std::deque<Onset> &new_onsets)
{
	std::vector<Agent *> new_agents;
	static Agent *highest_score_agent = agents[0];
	for (int i = 0; i < new_onsets.size(); ++i) {
		for (int j = agents.size() - 20; j < agents.size(); ++j) {
			// If this new onset is way past agent's last onset, this agent is invalid
			if (new_onsets[i].time_stamp - agents[j]->history.time_stamp > TIME_OUT) {
				agents.erase(agents.begin() + j);
			}
			// Otherwise create tolerance windows and check whether this onset can be added to this agent
			else {
				double tolerance_pre = 0.2 * agents[j]->interval;   // 20% of current interval
				double tolerance_post = 0.4 * agents[j]->interval;  // 40% of current interval

				while (agents[j]->prediction + tolerance_post < new_onsets[i].time_stamp) {
					agents[j]->prediction += agents[j]->interval;
				}

				// This onset falls within an existing agent's prediction interval
				if ((new_onsets[i].time_stamp >= agents[j]->prediction - tolerance_pre) &&
					(new_onsets[i].time_stamp < agents[j]->prediction + tolerance_post)) {
					// Now see if onset falls between inner and outer interval
					if (std::abs(agents[j]->prediction - new_onsets[i].time_stamp) > tolerance_inner) {
						Agent *new_agent = new Agent();
						new_agent->interval = agents[j]->interval;
						new_agent->prediction = agents[j]->prediction;
						new_agent->history = agents[j]->history;
						new_agent->score = agents[j]->score;
						new_agents.push_back(new_agent);
					}

					double error = new_onsets[i].time_stamp - agents[j]->prediction;
					double relative_error;
					if (error <= 0) relative_error = -error / tolerance_pre;
					else relative_error = error / tolerance_post;

					agents[j]->interval += error / CORRECTION_FACTOR;
					agents[j]->prediction = new_onsets[i].time_stamp + agents[j]->interval;
					agents[j]->history = new_onsets[i];
					agents[j]->score += (1 - relative_error / 2) * new_onsets[i].magnitude;
					if (agents[j]->score > highest_score_agent->score) {
						highest_score_agent = agents[j];
					}
				}
			}
		}
	}

	// Add new agents
	for (int i = 0; i < new_agents.size(); ++i) {
		agents.push_back(new_agents[i]);
	}

	// Remove duplicate agents
	for (int i = 0; i < agents.size(); ++i) {
		for (int j = i + 1; j < agents.size(); ++j) {
			if ((std::abs(agents[i]->interval - agents[j]->interval) < fs * 20 / 1000) &&
				(std::abs(agents[i]->prediction - agents[j]->prediction) < fs * 40 / 1000)) {
				if (agents[i]->score >= agents[j]->score) {
					delete agents[j];
					agents.erase(agents.begin() + j);
				}
				else {
					delete agents[i];
					agents.erase(agents.begin() + i);
				}
			}
		}
	}

	return highest_score_agent;
}

void new_clusters_tracking(std::vector<Cluster *> &clusters, std::vector<Cluster > &clusters_max, Agent* highest_score_agent) {

	Cluster* max_score_cluster = new Cluster();
	Cluster* secmax_score_cluster = new Cluster();
	Cluster* temp = new Cluster();
	bool max_cluster_change = false;
	bool secmax_cluster_change = false;
	Agent* highest_score_agent_temp;

	max_score_cluster = temp;
	secmax_score_cluster = temp;
	for (int t = 0; t < clusters.size(); t++) {
		if (clusters[t]->score > max_score_cluster->score) max_score_cluster = clusters[t];
	}

	for (int t = 0; t < clusters.size(); t++) {
		if (clusters[t] == max_score_cluster) continue;
		if (clusters[t]->score > secmax_score_cluster->score) secmax_score_cluster = clusters[t];
	}
	
	if (!clusters_max.empty()) {
		max_cluster_change = true;
		secmax_cluster_change = true;
		for (int t = 0; t < clusters_max.size(); t++) {
			if (abs(clusters_max[t].average_IOI - max_score_cluster->average_IOI) < cluster_width) {
				max_cluster_change = false;
			}
			if (abs(clusters_max[t].average_IOI - secmax_score_cluster->average_IOI) < cluster_width) {
				secmax_cluster_change = false;
			}
		}
		int tempo_1 = 6000 / max_score_cluster->average_IOI;
		//std::cout << "Clustering estimated BPM: " << tempo_1 << '\n';
		//std::cout<<"max_score_cluster:"<<max_score_cluster->average_IOI<<"\n";
		//std::cout<<"secmax_score_cluster:"<<secmax_score_cluster->average_IOI<<"\n";
		if (max_cluster_change) {
			clusters_max.push_back(*max_score_cluster);
			beat_tracking_initialisation(clusters_max, onsets, 1);
			highest_score_agent_temp = beat_tracking_update(onsets);
			if (highest_score_agent->score < highest_score_agent_temp->score) highest_score_agent = highest_score_agent_temp;
		}
		if (secmax_cluster_change) {
			clusters_max.push_back(*secmax_score_cluster);
			beat_tracking_initialisation(clusters_max, onsets, 1);
			highest_score_agent_temp = beat_tracking_update(onsets);
			if (highest_score_agent->score < highest_score_agent_temp->score) highest_score_agent = highest_score_agent_temp;
		}
	}

	else {
		clusters_max.push_back(*max_score_cluster);
		clusters_max.push_back(*secmax_score_cluster);
	}
}