/*
 * NAME: Matthew Wang
 * EMAIL: m5matthew@g.ucla.edu
 * ID: 504984273
 *
 * This is my header file for tls.c
 *
 */

#include <openssl/ssl.h>

void init_tcp(char* id, char* host, char* port_number );
SSL_CTX *init_tls();
void processCommands();
int serverIsReady();
void send_report(char* msg);
