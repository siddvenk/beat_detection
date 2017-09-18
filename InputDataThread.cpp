// File name: InputDataThread.cpp
//

#include "InputDataThread.h"

// ----------------- Function declarations necessary for UART ---------------------//
void ExitProgram(char * text);

struct timespec sleep_time_1, sleep_time_2;

void mySleep(uint16_t ms) 
{
	sleep_time_1.tv_sec = 0;
	sleep_time_1.tv_nsec = 1000000;	
	while (ms-- > 0) nanosleep(&sleep_time_1, &sleep_time_2);
}
uint16_t get_byte(uint8_t * value);
void     put_byte(uint8_t * value, uint16_t size);
void SendToRemote(uint8_t ch)
{
	put_byte(&ch, 1);
}

uint16_t ReceiveFromRemote(uint8_t *ch)
{
	while (get_byte(ch) == 0) mySleep(1);
	return 1;
}

int SendPacket(int sid, uint8_t *ptr, int len);
uint16_t GetPacket(uint16_t *sdid, uint8_t * buffer_ptr, uint16_t * len);
void initialize_data_source(long int baud_rate, unsigned sw_verbose);
// ----------------- Function declarations necessary for UART ---------------------//

const long int BAUD_RATE = 1500000;
uint16_t packet_sdid, packet_size;
uint8_t packet_buffer[256 + 1];

char text_buffer[256] = {0};

bool isSampleZero(int16_t &sample) {
	return ((sample < SILENCE_THRESHOLD_POSITIVE) &&
			(sample > SILENCE_THRESHOLD_NEGATIVE));
}

void collect_samples()
{
	int ctr = 0;
	int k = 0;
	bool first_fifo = true;
	long long silence_count = 0;
	int16_t value = 0;
	int16_t value_prev = 0;
	long long iteration = 0;
	uint16_t b_size = 0;

   	wiringPiSPISetup(0,500000);

	initialize_data_source(BAUD_RATE, 0);
	while (1) {

		//Collect samples, need B*H samples to proceed with calculations
		while (ctr < SAMPLE_QUEUE_SIZE) {
			GetPacket(&packet_sdid, &packet_buffer[0], &packet_size);
			uint8_t *ptr_from_receiver = packet_buffer;
			while (packet_size != 0) {
			 	text_buffer[tb_size++] = *ptr_from_receiver;
			 	packet_size--;
			 	ptr_from_receiver++;
			}
			tb_size = 0;
			std::string placeholder(text_buffer);
			value = stoi(placeholder);
			// used to keep track of silences
			if (isSampleZero(value_prev) && isSampleZero(value)) {
				silence_count++;
			}
			else {
				silence_count = 0;
			}
			if (silence_count > SILENCE_TO_RESET_SAMPLES) {
				std::cout << "Silence detected, program exiting\n";
			 	gpu_fft_release(fft); // Videocore memory lost if not freed !
			 	return;
			}
			if (first_fifo) {
				SAMPLE_QUEUE[ctr] = value;
			}
			else {
				SAMPLE_QUEUE.pop_front();
				SAMPLE_QUEUE.push_back(value);
			}
			value_prev = value;
			ctr++;
		}
		// Filled up the sample queue, so now adjust some values and calculate downsampled amplitude
		ctr = SAMPLE_QUEUE_SIZE - B;  // Specifies the overlap between successive calculations of RMS_AMPLITUDE[k]
		first_fifo = false;   // Only need to calculate B samples per cycle from here on out

		double temp = 0;
		for (int i = 0; i < SAMPLE_QUEUE_SIZE; ++i) {
			temp += (SAMPLE_QUEUE[i] * SAMPLE_QUEUE[i]);
		}
		RMS_AMPLITUE[k++] = sqrt((1/(H*B)) * temp);

		// When k is 100, we have 1 second worth of downsampled values, operating at ~1 second delay
		// call the onset_detection function/thread from here
		if (k == 100) {
			calculate_onsets(RMS_AMPLITUDE, k, iteration);
			iteration++;
		}
	}
}

// Assumes we have a full sample queue, k is 100
void calculate_onsets(std::deque<int16_t> &RMS_AMPLITUDE, int &k, long long &iteration) 
{
	for (int i = 3; i < k; ++i) {
		double A = 0;
		double B = 0;
		double C = 0;
		double D = 0;
		double E = 0;

		for (int j = i - 3; j <= i; ++j) {
			A += j * B * RMS_AMPLITUDE[j] / FS;
			B += RMS_AMPLITUDE[j];
			C += j * B / FS;
			D += (RMS_AMPLITUDE[j] * RMS_AMPLITUDE[j]);
			E += RMS_AMPLITUDE[j];
		}
		SLOPES[i-3] = (A - B*C) / (4*D - E*E);
	}

	// Peak detection using 100 amplitude values and 97 slope values
	for (int i = 3; i < k - 3; ++i) {
		bool check1 = true;
		bool check2 = true;

		if (RMS_AMPLITUDE[i] < AMPLITUDE_THRESHOLD) {
			check1 = false;
		}
		if (abs(SLOPES[i]) < SLOPE_THRESHOLD) {
			check2 = false;
		}

		if (check1 && check2) {
			ONSETS.push_back(Onset((iteration * (k-3) + i), RMS_AMPLITUDE[i]));
		}
	}
	// Shift amplitued values by 97 left
	RMS_AMPLITUDE[0] = RMS_AMPLITUDE[k-1];
	RMS_AMPLITUDE[1] = RMS_AMPLITUDE[k-2];
	RMS_AMPLITUDE[2] = RMS_AMPLITUDE[k-3];
	k = 3;

	// Need to call a clustering function/thread from here
}
