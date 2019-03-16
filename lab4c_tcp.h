/*
 * NAME: Matthew Wang
 * EMAIL: m5matthew@g.ucla.edu
 * ID: 504984273
 *
 * This is my header file for lab4c_tcp.c
 *
 */

#include <mraa/aio.h>

void _shutdown();
void readTempAndReport(mraa_aio_context temp_sensor);
void executeCommand(char* str);
int hasPeriod(char* str);
void log_msg(char* report);
void exit_arg(char* msg);
void exit_error(char* msg);
