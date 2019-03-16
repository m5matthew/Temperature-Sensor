/*
 * NAME: Matthew Wang
 * EMAIL: m5matthew@g.ucla.edu
 * ID: 504984273
 *
 * This file sets up TCP TLS connection with the server.
 *
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h> //htons
#include <string.h>
#include <poll.h>
#include <stdlib.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "lab4c_tls.h"
#include "tls.h"

static int sockfd = 0;
static SSL *ssl_client=0;

/* Initializes tcp connection with server */
/* Most of this function is derived from the man page of getaddrinfo()
	https://manpages.debian.org/jessie/manpages-dev/getaddrinfo.3.en.html#Client_program
	*/
void init_tcp(char* id, char* host, char* port_number )
{
	struct addrinfo attr;
	struct addrinfo *result;

	memset(&attr, 0, sizeof(struct addrinfo));
	attr.ai_family = AF_UNSPEC;
	attr.ai_socktype = SOCK_STREAM; // Specifies that we want TCP connection
	attr.ai_flags = 0;
	attr.ai_protocol = 0;

	/* Convert human readable host to IP address */
	if(getaddrinfo(host, port_number, &attr, &result) != 0){
		exit_error("Failed to get address info");
	}

	/* Sets up socket */
	sockfd= socket(result->ai_family, result->ai_socktype, 0);
	if(sockfd == -1){
		exit_error("Failed to create socket");
	}

	/* Connects with server */
	if(connect(sockfd, result->ai_addr, result->ai_addrlen) < 0){
		exit_error("Failed to connect");
	}

	/* Set up TLS */
	SSL_CTX* ssl_context = init_tls();
	ssl_client = SSL_new(ssl_context);
	if(ssl_client == NULL){
		exit_error("Error creating new SSL structure");
	}
	if(!SSL_set_fd(ssl_client,sockfd)){
		exit_error("Error connecting ssl with socket");
	}
	if(SSL_connect(ssl_client)!=1){
		exit_error("Error initiating TLS handshake");
	}

	/* Makes sure we dont block when reading from server */
	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	/* Sends ID to server */
	char id_string[13];
	sprintf(id_string, "ID=%s\n", id);
	log_msg(id_string);
	if( SSL_write(ssl_client, id_string, strlen(id_string)) <= 0 ){
		exit_error("Failed to write ID to server");
	}
}

/* Initialize TLS */
SSL_CTX* init_tls()
{
	SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
	SSL_CTX *ssl_context = SSL_CTX_new(TLSv1_client_method());
	if(ssl_context ==NULL){
		exit_error("Error creating new SSL_CTX object");
	}
	return ssl_context;
}

/* Sends a report to the server */
void send_report(char* msg)
{
	if(SSL_write(ssl_client, msg, strlen(msg)) <= 0){
		exit_error("Failed to write report to server");
	}
}

/* Polls server. Returns 1 if ready, 0 if not */
int serverIsReady()
{
	struct pollfd server;
	server.fd = sockfd;
	server.events = POLLIN;
	poll(&server, 1, 50); // poll stdin every ~50ms

	if((server.revents & POLLIN) == POLLIN){
		return 1;
	}
	else{
		return 0;
	}
}

/* Processes commands inputted from server*/
void processCommands()
{
	char buff[1];
	char *command = malloc(sizeof(char));
	if(!command){
		exit_error("malloc() failed");
	}
	int ret = 0;
	int i = 0;
	do{
		// read in one byte at a time
		ret = SSL_read(ssl_client, buff, 1);

		// store input string in 'command'
		command[i] = buff[0];
		i++;
		command = realloc(command, (i+1)*sizeof(char));
		if(!command){
			exit_error("realloc() failed");
		}
		// read until we reach newline, which means we've processed an entire command
		if(buff[0] == '\n' && command[0] != '\n'){
			command[i] = '\0';
			i = 0;
			log_msg(command);
			executeCommand(command);
		}

	}while(ret > 0);
	free(command);
	return;
}
