/*
 * NAME: Matthew Wang
 * EMAIL: m5matthew@g.ucla.edu
 * ID: 504984273
 *
 * This is my source code for CS111 Lab4C Part 2.
 *
 */

 #include <stdio.h>
 #include <mraa/aio.h>
 #include <getopt.h>
 #include <signal.h>
 #include <stdlib.h>
 #include <time.h>
 #include <fcntl.h>
 #include <string.h>

 #include "util.h"
 #include "tls.h"
 #include "lab4c_tls.h"

 #define EXIT_BADARG 1
 #define EXIT_FAIL 2
 #define ID 3
 #define HOST 4
 #define PERIOD 5
 #define SCALE 6
 #define LOG 7

 sig_atomic_t volatile exit_flag = 0;	/* Gets set if button is pressed */
 int host_id_flag = 0; 	  /* Used to detect mandatory arguments host and id */
 int farenheit_flag = 1;	  /* Default temp is in farenheit */
 int stop_flag = 0;		  /* Set when STOP is entered on STDIN */
 int log_flag = 0;		  /* Set when logfile is specifeid on CLI */
 int time_interval = 1;    /* Default time interval is 1 sec */
 char* logfile = "";	      /* Name of logfile */
 int logfd = 0;			  /* File descriptor of logfile */


 /* Error helper function */
 void exit_error(char* msg)
 {
 	fprintf(stderr,"ERROR: %s\n", msg);
 	exit(EXIT_FAIL);
 }

 void exit_arg(char* msg)
 {
 	fprintf(stderr, "ERROR: %s\n", msg);
 	exit(EXIT_BADARG);
 }

 /* Write to logfile */
 void log_msg(char* report)
 {
 	if(log_flag){
 		if(write(logfd,report,strlen(report)) < 0){	// write to logfile
 			exit_error("Error writing to logfile");
 		}
 	}
 }

 /* Detects PERIOD=[NUM] in str */
 int hasPeriod(char* str)
 {
 	// return if length less than 6
 	if(strlen(str) < 6){
 		return 0;
 	}

 	// get substring of first 6 letters
 	char substr[8];
 	strncpy(substr,str,7);
 	substr[7] = '\0';

 	// check if those letters are PERIOD= and if there is an argument
 	if((strcmp("PERIOD=",substr) == 0) && (str[7] != '\n')){
 		return 1;
 	}
 	return 0;
 }
 /* Executes command given as input */
 void executeCommand(char* str)
 {
 	if(strcmp(str, "SCALE=F\n") == 0){
 		farenheit_flag = 1;
 	}
 	else if(strcmp(str, "SCALE=C\n") == 0){
 		farenheit_flag = 0;
 	}
 	else if(hasPeriod(str)){
 		int arg = atoi(str+7);
 		if(arg != 0){
 			time_interval = arg;
 		}
 	}
 	else if(strcmp(str, "STOP\n") == 0){
 		stop_flag = 1;
 	}
 	else if(strcmp(str, "START\n") == 0){
 		stop_flag = 0;
 	}
 	else if(strcmp(str, "OFF\n") == 0){
 		exit_flag = 1;
 	}
 }

 /* Reads temperature and sends to sever */
 void readTempAndReport(mraa_aio_context temp_sensor)
 {
 	// read temperature
 	double temperature = celsius(mraa_aio_read(temp_sensor));
 	if(farenheit_flag){
 		temperature = farenheit(temperature);
 	}

 	// get timestamp
 	char timestamp[10];
 	getTimestamp(timestamp);

 	// report and log
 	char report[20];
 	sprintf(report, "%s %.1f\n", timestamp, temperature);
 	send_report(report);
 	log_msg(report);

 }

 /* Shutdowns and reports message to logfile and server */
 void _shutdown()
 {
 	// get timestamp
 	char timestamp[10];
 	getTimestamp(timestamp);

 	// generate report
 	char report[20];
 	sprintf(report,"%s SHUTDOWN\n", timestamp);

 	// report, log, and exit
 	send_report(report);
 	log_msg(report);
 	exit(0);
 }

 /* --------------
  * MAIN FUNCTION
  * --------------
  * Main function of the program
  *
  */
 int main(int argc, char **argv)
 {
     static struct option long_options[] ={
         {"period", required_argument, 0, PERIOD},
         {"scale", required_argument, 0, SCALE},
         {"log", required_argument, 0, LOG},
 		{"id", required_argument, 0, ID},
 		{"host", required_argument, 0, HOST},
         {0,0,0,0}
     };

 	char* id;
 	char* host;
 	char* port_number;

     /* Process CLI arguments */
     while (1)
     {
         int arg = getopt_long(argc, argv,"",long_options, NULL);
         if(arg == -1)
             break;

         switch(arg){
             case PERIOD:
                 time_interval = atoi(optarg);
 				if(time_interval == 0){
 					exit_arg("Incorrect --period argument");
 				}
                 break;
             case SCALE:
 				if(optarg[0] != 'C' && optarg[0] != 'F')
 					exit_arg("Incorret argument to --scale");
                 farenheit_flag = (optarg[0] == 'C' ? 0 : 1);
                 break;
 			case LOG:
 				logfile = optarg;
 				log_flag = 1;
 				break;
 			case ID:
 				if(strlen(optarg) != 9){
 					exit_arg("ID must be 9 digits");
 				}
 				id = optarg;
 				if(atoi(id) == 0){
 					exit_arg("Invalid ID");
 				}
 				host_id_flag |= 0x01;
 				break;
 			case HOST:
 				host = optarg;
 				host_id_flag |= 0x02;
 				break;
             default:
                 exit(EXIT_BADARG);
                 break;
         }
     }

 	/* Make sure that the mandatory arguments were specifeid*/
 	if(optind >= argc){
 		exit_arg("Need to specify port number");
 	}
 	else if(host_id_flag != 0x03){
 		exit_arg("Need to specifiy host and id");
 	}

 	/* Extract port_number from CLI */
 	port_number = argv[optind];
 	if(atoi(port_number) == 0 || (optind+1) < argc)
 		exit_arg("Invalid or too many port numbers");

 	/* Initialize button and temperature sensor */
     mraa_aio_context temp_sensor;
     temp_sensor = mraa_aio_init(1);
     if(temp_sensor == NULL){
 		exit_error("Failed to initialize AIO");
     }


 	/* Open logfile if specified */
 	if(log_flag){
 		logfd = open(logfile, O_CREAT | O_WRONLY | O_APPEND, 0666);
 		if(logfd < 0){
 			exit_error("Failed to open logfile");
 		}
 	}

	
 	/* Init tcp connection */
 	init_tcp(id, host, port_number);

     while(1)
 	{
 		/* Send temperature stats to server */
         if(!stop_flag){
 			readTempAndReport(temp_sensor);
 		}

 		time_t start = time(NULL);
 		time_t end = time(NULL);

 		/* Poll server for commands */
 		while(1)
 		{
 			if((end-start) >= time_interval)
 				break;
 			/* If button was pressed, exit */
 			if(exit_flag){
 				_shutdown();
 			}
 			if(serverIsReady()){
 				processCommands();
 			}
 			end = time(NULL);
 		}

     }

     mraa_aio_close(temp_sensor);

     exit(0);
 }
