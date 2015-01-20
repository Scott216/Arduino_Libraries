Source: http://playground.arduino.cc/Main/Statistics


Statistic();		// constructor
void clear();		// reset all counters
void add(float);	// add a new value
long count();		// # values added
float sum();		// total
float minimum();	// minimum
float maximum();	// maximum
float average();	// average
float pop_stdev();	// population std deviation
float unbiased_stdev();	// unbiased std deviation 


Internally the library does not record the individual values, only the count, sum and the squared sum (sum*sum), minimum and maximum. These five are enough to calculate the average and stdev. The nice part is that it does not matter if one adds 10, 100 or 1000 values.

