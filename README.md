# Beat Detection
Senior Design Project Code

This repository holds the code for both TI C5515 and raspberry pi devices. The main sections and algorithms are described in the section below. 

Our system involves 5 main steps: collecting and transmitting samples, onset detection, tempo induction, beat prediction, and outputting information. We will now expand on the functions of each stage, and present algorithms in the next section.

Collecting and Transmitting Samples
We used a TIC5515 to collect our samples, and transmit them to the raspberry pi. A two way 3.5mm audio cord was connected from the source of the music, to the audio in port on the TIC5515. The TIC5515 runs a modified version of the lab 6 code to collect the audio samples at a sampling rate of 48 kHz. We require that the song be played at maximum volume on the device to reduce effects of noise, and get the most accurate results. Every time we collect a sample, we send it to the raspberry pi using the UART communication protocol. 

Onset Detection
The goal of the onset detection step is to analyze the incoming input samples, and determine the locations and magnitudes of note onsets in the song. An onset refers to the beginning of a musical note or sound, and onset locations are usually a strong indicator of possible beat locations. After collecting the samples on the raspberry pi, we compute differences in spectral energy between successive frames of data, and using various thresholds that we will cover in the next section determine the locations and magnitudes of onsets. These onsets are then sent to our tempo induction step.

Tempo Induction
The goal of the tempo induction step is to come up with possible tempos for the song based on the onsets. This is done by taking differences between the onset times in memory, clustering similar differences together, and scoring the clusters based on various factors. The 2 highest scoring clusters are sent to the beat prediction step.

Beat Prediction
The goal of the beat prediction step is to use objects called ‘agents’ to determine whether onsets represent beat locations. We use the information from the tempo induction and onset detection stages to create, update, and delete agents. We score the agents currently in memory based on how well an onset fits its current tempo and beat prediction. At the end of this step we send the highest scoring agent to the output stage.

Output stage
The goal of this stage is to output the tempo information to the raspberry pi console, and the beat locations to the blue LED in time with the beats of the song. We use the highest scoring agent’s data to accomplish this. 

