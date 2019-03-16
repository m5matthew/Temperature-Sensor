/*
 * NAME: Matthew Wang
 * EMAIL: m5matthew@g.ucla.edu
 * ID: 504984273
 *
 * This file contains functions that help with general computation
 *
 */
#include <math.h>
#include <time.h>

/* Converts sensor reading to celsius
 * source: http://wiki.seeedstudio.com/Grove-Temperature_Sensor_V1.2/ */
double celsius(int value)
{
	int R0 = 100000;
	int B = 4275;
	float R = 1023.0/value-1.0;
	R = R0*R;
	return 1.0/(log(R/R0)/B+1/298.15)-273.15;
}

/* Converts celsius to farenheit */
double farenheit(double celsius)
{
	return celsius * (9.0/5.0) + 32.0;
}

/* Gets current timestamp */
void getTimestamp(char*buff)
{
	time_t time_in_seconds = time(NULL);
	struct tm *time_info = localtime(&time_in_seconds);
	strftime(buff, 10, "%H:%M:%S", time_info);
	return;
}
