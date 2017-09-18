FFT = mailbox.c gpu_fft.c gpu_fft_base.c gpu_fft_twiddles.c gpu_fft_shaders.c
THREADS = InputDataThread.cpp OnsetDetectionThread.cpp beat_clustering.cpp beat_tracking.cpp
MISC = Interface_to_Linux.c WP_packet_support.c

MAIN = BeatDetectionDriver.cpp

EXEC = BeatDetection

HEAD = gpu_fft.h FIFO_builder.h globals.h InputDataThread.h mailbox.h Objects.h OnsetDetectionThread.h 

F = -lrt -lm -ldl -lwiringPi -pthread -std=c++11 -O3

all:	$(EXEC) 
$(EXEC): $(FFT) $(THREADS) $(HEAD) $(MAIN) $(MISC)
	g++ $(F) $(FFT) $(THREADS) $(MISC) $(MAIN) -o $(EXEC)

clean:
	rm -f *.o