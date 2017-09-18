#include "OnsetDetectionThread.h"

// declare function variables here
float data_buffer_current[2 * FFT_SIZE];
float data_buffer_prev[2 * FFT_SIZE];

double detection_function[NUM_WINDOWS];
double detection_function_g[NUM_WINDOWS];

bool first_fft = true;

int counter = 0;
long down_sampled_time_stamp = 0;

std::vector<Cluster > clusters_max;
Agent* highest_score_agent = nullptr;
bool agents_start = false;

void onset_detection(std::deque<float> &samples) {

	// copy the input samples to a local buffer, release lock, and alert input thread
	for (int i = 0; i < FFT_SIZE; ++i) {
		data_buffer_current[2 * i] = samples[i] * HAMMING[i];
		data_buffer_current[2 * i + 1] = 0;
	}

	for (int i = 0; i < FFT_SIZE; i++)
	{
		fft->in[i].re = data_buffer_current[2*i];
		fft->in[i].im = data_buffer_current[2*i+1];
	}

    gpu_fft_execute(fft); // call one or many times
    for (int t = 0; t < FFT_SIZE; t++) {
		data_buffer_current[t*2] = fft->out[t].re;
		data_buffer_current[t*2+1] = fft->out[t].im;
	}

	if (first_fft) {
		first_fft = false;
		for (int i = 0; i < FFT_SIZE; ++i) {
			data_buffer_prev[2*i] = data_buffer_current[2*i];
			data_buffer_prev[2*i + 1] = data_buffer_current[2*i + 1];
		}
		return;
	}
	else {
		double sum = 0;
		double difference = 0;
		// Only need half the fft since real valued signal is symmetric around the midpoint
		for (int i = 0; i < FFT_SIZE; ++i) {
			double mag_prev = sqrt(data_buffer_prev[2*i] * data_buffer_prev[2*i] + data_buffer_prev[2*i+1] * data_buffer_prev[2*i+1]);
			double mag_cur = sqrt(data_buffer_current[2*i] * data_buffer_current[2*i] + data_buffer_current[2*i+1] * data_buffer_current[2*i+1]);
			difference = mag_cur - mag_prev;
			sum += (difference + abs(difference)) / 2;
		}
		//std::cout << "detection_function assignment: ";
		detection_function[counter] = sum;
		//std::cout << detection_function[counter] << "\n";
		counter++; down_sampled_time_stamp++;
		for (int i = 0; i < FFT_SIZE; ++i) {
			data_buffer_prev[2*i] = data_buffer_current[2*i];
			data_buffer_prev[2*i + 1] = data_buffer_current[2*i + 1];
		}
		// Check to see if have half second of data yet
		if (counter == NUM_WINDOWS) {
			// convert using mean and variance
			std::deque<Onset> new_onsets;
			double mean = 0;
			double total_sum = 0;
			for (int i = 0; i < NUM_WINDOWS; ++i) {
				total_sum += detection_function[i];
			}
			mean = total_sum / NUM_WINDOWS;
			double variance = 0;
			double standard_deviation = 0;
			for (int i = 0; i < NUM_WINDOWS; ++i) {
				variance += ((detection_function[i] - mean) * (detection_function[i] - mean));
			}
			standard_deviation = sqrt(variance / NUM_WINDOWS);

			// Normalize using zero mean and unit variance
			for (int i = 0; i < NUM_WINDOWS; ++i) {
				detection_function[i] = (detection_function[i] - mean) / standard_deviation;
			}

			//Calculate the g_function
			detection_function_g[0]=0;

			for (int i = 1; i < NUM_WINDOWS; ++i){
				float max_g = alpha * detection_function_g[i-1] + (1 - alpha) * detection_function[i];
				if(max_g > detection_function[i]){
					detection_function_g[i] = max_g;
				}
				else{
					detection_function_g[i] = detection_function[i];
				}
			}

			// Peak picking algorithm
			float window_mean = 0;
			float window_temp[m*w+w+1];
			float window_max = 0;
			int check;

			for(int n = w*m; n < NUM_WINDOWS-w; ++n){
				bool check1 = true;
				bool check2 = true;
				bool check3 = true;

				// first check
				for (int k = n-w; k <= n+w; ++k) {
					if (detection_function[n] < detection_function[k]) {
						check1 = false;
						break;
					}
				}

				// second check
				float temp = 0;
				for (int k = n-w*m; k <= n+w; ++k) {
					temp += detection_function[k];
				}
				temp = (temp / (m*w + w + 1)) + delta;

				if (detection_function[n] < temp) {
					check2 = false;
				}

				// third check
				if (detection_function[n] < detection_function_g[n-1]) {
					check3 = false;
				}

				if (check1 && check2 && check3) {
					(detection_function[n] < 0) ? (detection_function[n] *= -1) : (detection_function[n] = detection_function[n]);
					new_onsets.push_back(Onset((down_sampled_time_stamp - (NUM_WINDOWS - n) + 1), detection_function[n]));
				}

			}
			// // Now shift the detection_function by 48 to the left
			for (unsigned i = 0; i < 13; ++i) {
				detection_function[i] = detection_function[i + 48];
			}
			counter = 13;

			// Call Cllustering
			//std::cout << "Start Clustering\n";
			clustering_real_time(new_onsets);
			// int tempo = 6000 / averageIOI;
			// std::cout << "Estimated Tempo: " << tempo << '\n';

			if (agents_start == true)
				highest_score_agent = beat_tracking(new_onsets);

			if (onsets.size() >= 20) {
				new_clusters_tracking(clusters, clusters_max, highest_score_agent);

				if (agents_start == false)
				{
					beat_tracking_initialisation(clusters_max, onsets, 2);
					highest_score_agent = beat_tracking(new_onsets);
					agents_start = true;
				}
				// for (int i = 0; i < clusters.size(); ++i) {
				// 		//std::cout << "Cluster interval: " << clusters[i]->average_IOI << " Cluster score: " << clusters[i]->score << '\n';
				// }
			}
			if (highest_score_agent != nullptr) {
				int tempo = 6000 / highest_score_agent->interval;
				while (tempo > 240) {
					tempo = tempo / 2;
				}
				while (tempo < 30) {
					tempo = tempo * 2;
				}
				//for (int k = 0; k < onsets.size(); k++) std::cout<<"onsets_location"<<onsets[k].time_stamp<<"\n";
				std::cout << "Highest scoring agent indicates BPM:  " << tempo << '\n';
			}
			else
				std::cout << "No Agent found yet. Too early in song\n";
		}
	}
	//std::cout << "Exiting onset\n";
	return;
}