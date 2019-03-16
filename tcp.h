/*
 * NAME: Matthew Wang
 * EMAIL: m5matthew@g.ucla.edu
 * ID: 504984273
 *
 * This is my header file for tcp.c
 *
 */

void init_tcp(char* id, char* host, char* port_number );
void processCommands();
int serverIsReady();
void send_report(char* msg);
