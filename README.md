# Calc-Sample-Interval

Background
-----------
Calculating data sample intervals on a non-realtime system using mode( most common) determination.

The code was developed to correct a problem which occurred when attempting to determine to data sample
interval transmitted using both UDP and RS485, on a non-realtime system. It was found that an accurate
value could not be obtained from mean or average calculations due to excessive jitter in the timing
measurement. This was for both Linux using clock_gettime( CLOCK_REALTIME) and Windows using 
QueryPerformanceCounter.

However it was found that an accurate measurement could be determined if the mode, the most common
value was determined. 

Code
----
The code was developed for Linux but could be modified for Windows. I haven't used a conditional
compilation for both as it looks too untidy.
The calculations are performed using the class CCalcAutoRate; files calctime.cpp calctime.h.

Folders
-------
code: This contains the class implementation and definition files calctime.cpp calctime.h.

calctime codeblocks: This folder is a codeblocks project which demonstrates the calculation
                     of the mean using a UDP loopback to simulate the original data
                     acquisition problem.
                     
calctime make: This code in this folder runs the same simulation as above, but can be built
               from a terminal using the Makefile present.
                     
                   


