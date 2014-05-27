/*
File:   ViamSonus.cpp
Author: J. Ian Lindsay
Date:   2014.05.19

Copyright (C) 2014 J. Ian Lindsay
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


This is an example sketch that shows the basic operation of the analog router PCB.
 
*/

#include "Logger/Logger.h"
#include "AudioRouter/AudioRouter.h"
#include "i2c-adapter/i2c-adapter.h"

#define VERSION_STRING  "0.0.1"
#define HOST_BAUD_RATE  9600


const uint8_t SWITCH_ADDR = 0x76;
const uint8_t POT_0_ADDR  = 0x50;
const uint8_t POT_1_ADDR  = 0x51;


I2CAdapter *i2c = NULL;
AudioRouter *audio_router = NULL;


extern IansLogger logger;

#ifdef ARDUINO
/****************************************************************************************************
* Entry-point for teensy3...                                                                        *
****************************************************************************************************/

void setup() {
	// Setup i2c...
	i2c = new I2CAdapter();
	
	// Build the router object...
	audio_router = new AudioRouter(SWITCH_ADDR, POT_0_ADDR, POT_1_ADDR);

	Serial.begin(HOST_BAUD_RATE);                           // Setup host communication.
}


void loop() {
	
	while (Serial.available() < 1) {
	}
	c = Serial.read();
	
	switch (c) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			break;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			break;
		case '':
			break;
		case '':
			break;
		case '':
			break;
		case '':
			break;
	}
}

#else


/****************************************************************************************************
* Entry-point for a linux build...                                                                  *
****************************************************************************************************/

#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <iostream>

#include <fcntl.h>
#include <sys/signal.h>
#include <errno.h>
#include <termios.h>

#include <time.h>




/**
* The help function. We use printf() because we are certain there is a user at the other end of STDOUT.
*/
void printUsage() {
	printf("==================================================================================\n");
	printf("Bus and channel selection:\n");
	printf("==================================================================================\n");
	printf("    --i2c-dev     Specify the i2c device to use.\n");
	printf("-i  --input       input pin (0-11)\n");
	printf("-o  --output      output pin (0-7)\n");
	printf("\n");

	printf("==================================================================================\n");
	printf("Options that operate on channels:\n");
	printf("==================================================================================\n");
	printf("-r  --route       Route the given input to the given output.\n");
	printf("-u  --unroute     Unroute the given input from the given output(s). If no output\n");
	printf("                   is specified, we will unroute the given input from all outputs.\n");
	printf("-v  --volume      Specify the value of the linear potentiometer (0-255).\n");
	printf("                   If no output is specified, we will adjust all outputs.\n");
	printf("-m  --mute        Mutes a specified output channel. If no output channel is\n");
	printf("                   specified, we will mute all outputs. Note that muting an output\n");
	printf("                   has no effect on the route.\n");
	printf("\n");

	printf("==================================================================================\n");
	printf("Meta:\n");
	printf("==================================================================================\n");
	printf("-v  --version     Print the version and exit.\n");
	printf("-h  --help        Print this output and exit.\n");
	printf("-s  --status      Read and print the present condition of the PCB.\n");
	printf("    --reset       Reset the PCB back to it's power-on state.\n");
	printf("    --enable      Enable a PCB that was previously disabled.\n");
	printf("    --disable     Disable the PCB. Mutes all outputs.\n");
	printf("\n\n");
}



int main(int argc, char *argv[]) {
	char operation   = '.';     // No operation.
	uint8_t volume       = 128;
	uint8_t input_chan   = 255;
	uint8_t output_chan  = 255;
	
	logger.setVerbosity(7);


	// Parse through all the command line arguments and flags...
	// Please note that the order matters. Put all the most-general matches at the bottom of the loop.
	for (int i = 1; i < argc; i++) {
		if ((strcasestr(argv[i], "--help")) || ((argv[i][0] == '-') && (argv[i][1] == 'h'))) {
			printUsage();
			exit(0);
		}
		else if ((strcasestr(argv[i], "--version")) || ((argv[i][0] == '-') && (argv[i][1] == 'v'))) {
			printf("%s v%s\n\n", argv[0], VERSION_STRING);
			exit(0);
		}
		else if (argc - i >= 2) {    // Compound arguments go in this case block...
			if (strcasestr(argv[i], "--i2c-dev")) {
				i2c = new I2CAdapter(atoi(argv[++i]));          // Fire up the i2c interface...
				i2c->setDebug(true);
			}
			else if (strcasestr(argv[i], "--volume") || ((argv[i][0] == '-') && (argv[i][1] == 'v'))) {
				int temp_vol = atoi(argv[++i]);
				if ((temp_vol > 255) || (temp_vol < 0)) {
					printf("Volume needs to be specified as an integer between 0 and 255.\n");
					exit(0);
				}
				volume = (uint8_t) temp_vol;
				operation = 'v';
			}
			else if (strcasestr(argv[i], "--input") || ((argv[i][0] == '-') && (argv[i][1] == 'i'))) {
				int temp = atoi(argv[++i]);
				if ((temp > 11) || (temp < 0)) {
					printf("Input channel must be between 0 and 11.\n");
					exit(0);
				}
				input_chan = (uint8_t) temp;
			}
			else if (strcasestr(argv[i], "--output") || ((argv[i][0] == '-') && (argv[i][1] == 'o'))) {
				int temp = atoi(argv[++i]);
				if ((temp > 7) || (temp < 0)) {
					printf("Output channel must be between 0 and 7.\n");
					exit(0);
				}
				output_chan = (uint8_t) temp;
			}
			else {
				i++;
			}
		}
		else if (strcasestr(argv[i], "--enable")) {
			operation = 'e';
		}
		else if (strcasestr(argv[i], "--disable")) {
			operation = 'd';
		}
		else if (strcasestr(argv[i], "--reset")) {
			operation = 'x';
		}
		else if (strcasestr(argv[i], "--route") || ((argv[i][0] == '-') && (argv[i][1] == 'r'))) {
			operation = 'r';
		}
		else if (strcasestr(argv[i], "--status") || ((argv[i][0] == '-') && (argv[i][1] == 's'))) {
			
			operation = 's';
		}
		else if (strcasestr(argv[i], "--unroute") || ((argv[i][0] == '-') && (argv[i][1] == 'u'))) {
			operation = 'u';
		}

		else {
			printf("Unhandled argument: %s\n", argv[i]);
			printUsage();
			exit(1);
		}
	}

	
	if ((i2c != NULL) && (i2c->busOnline())) {
		audio_router = new AudioRouter(SWITCH_ADDR, POT_0_ADDR, POT_1_ADDR);
		// Since this program will do its job and exit immediately (taking the
		//   state of the switch with it), we need to instruct the class to not
		//   disable the hardware when the program exits.
		audio_router->preserveOnDestroy(true);
		
		int8_t result = 0;
		char *temp_str;
		switch (operation) {
			case 'r':
				result = audio_router->route(output_chan, input_chan);
				break;
			case 'u':
				if (input_chan == 255) {
					result = audio_router->unroute(output_chan);
				}
				else {
					result = audio_router->unroute(output_chan, input_chan);
				}
				break;
			case 's':
				temp_str = (char*) alloca(1024);
				result = audio_router->status(temp_str);
				printf("%s\n", temp_str);
				break;
			case 'e':
				result = audio_router->enable();
				break;
			case 'd':
				result = audio_router->disable();
				break;
			case 'v':
				if (output_chan == 255) {
					for (int i = 0; i < 8; i++) {
						result = audio_router->setVolume(i, volume);
					}
				}
				else {
					result = audio_router->setVolume(output_chan, volume);
				}
				break;
			case 'x':
				//if (audio_router->enabled()) {
					audio_router->disable();
				//}
				result = audio_router->enable();
				break;
			case '.':
				// No operation selected.
				printf("Nothing to do.\n");
				exit(0);
				break;
			default:
				printf("Unhandled case.\n");
				break;
		}
		
		switch (result) {
			case AudioRouter::AUDIO_ROUTER_ERROR_NO_ERROR:
				printf("Operation completed with success.\n");
				break;
			case AudioRouter::AUDIO_ROUTER_ERROR_INPUT_DISPLACED:
				printf("Operation completed with success, but we displaced a previously established route.\n");
				break;
			case AudioRouter::AUDIO_ROUTER_ERROR_BAD_COLUMN:
				printf("Error: Output channel is out of range.\n");
				break;
			case AudioRouter::AUDIO_ROUTER_ERROR_BAD_ROW:
				printf("Error: Input channel is out of range.\n");
				break;
			case AudioRouter::AUDIO_ROUTER_ERROR_UNROUTE_FAILED:
				printf("Error: Failed to unroute the given channels.\n");
				break;
			default:
				printf("Unhandled case: (%d).\n", result);
				break;
		}
	}
	else {
		printf("You need to supply a valid i2c device.\n");
	}
	
	exit(0);
}


#endif
