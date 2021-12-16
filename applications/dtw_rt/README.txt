DTW = Dynamic Timing Warping algorithm for patter recognition. It is used measuring similarity between 
two temporal sequences, which may vary in speed. For instance, similarities in walking could be detected 
using DTW, even if one person was walking faster than the other, or if there were accelerations and 
decelerations during the course of an observation. DTW has been applied to temporal sequences of video, 
audio, and graphics data — indeed, any data that can be turned into a linear sequence can be analyzed 
with DTW. A well known application has been automatic speech recognition, to cope with different speaking 
speeds. Other applications include speaker recognition and online signature recognition. Also it is seen 
that it can be used in partial shape matching application.

The algorithm has a recognizer task, which acts as master. It provides the sample to be recognized according
to the patterns stored in the bank task. In real-life the bank task will implement a I/O interface to gather
the patterns from a bigger memory.

The recognizer initializes the threads, p1, p2.... It sends the same sample to all threads. Each thread then runs
the DTW algoritm comparing it to a pattern in the bank. Each thread is related to block of patterns in the bank.
At the end, each thread sends the result to recognizer, which compares which one achieves the best results.