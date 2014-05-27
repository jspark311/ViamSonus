/*
File:   AudioRouter.cpp
Author: J. Ian Lindsay
Date:   2014.03.10


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

*/

#include "AudioRouter.h"
#include "../ISL23345/ISL23345.h"
#include "../ADG2128/ADG2128.h"

#include "../Logger/Logger.h"
extern IansLogger logger;

#include <string.h>

/*
* To facilitate cheap (2-layer) PCB layouts, the pots are not mapped in an ordered manner to the
* switch outputs. This lookup table allows us to forget that fact.
*/
const uint8_t AudioRouter::col_remap[8] = {0x03, 0x02, 0x01, 0x00, 0x07, 0x06, 0x05, 0x04};



/*
* Constructor. Here is all of the setup work. Takes the i2c addresses of the hardware as arguments.
*/
AudioRouter::AudioRouter(uint8_t cp_addr, uint8_t dp_lo_addr, uint8_t dp_hi_addr) {
	i2c_addr_cp_switch = cp_addr;
	i2c_addr_dp_lo = dp_lo_addr;
	i2c_addr_dp_hi = dp_hi_addr;
	
    cp_switch    = new ADG2128(cp_addr);
    dp_lo        = new ISL23345(dp_lo_addr);
    dp_hi        = new ISL23345(dp_hi_addr);
    
    for (uint8_t i = 0; i < 12; i++) {   // Setup our input channels.
      inputs[i] = (CPInputChannel*) malloc(sizeof(CPInputChannel));
      inputs[i]->cp_row   = i;
      inputs[i]->name     = NULL;
    }

    for (uint8_t i = 0; i < 8; i++) {    // Setup our output channels.
      outputs[i] = (CPOutputChannel*) malloc(sizeof(CPOutputChannel));
      outputs[i]->cp_column = col_remap[i];
      outputs[i]->cp_row    = NULL;
      outputs[i]->name      = NULL;
      outputs[i]->dp_dev    = (i < 4) ? dp_lo : dp_hi;
      outputs[i]->dp_reg    = i % 4;
      outputs[i]->dp_val    = 128;
    }
    
    if (init() != AUDIO_ROUTER_ERROR_NO_ERROR) {
    	logger.unified_log(__PRETTY_FUNCTION__, LOG_ERR, "Tried to init AudioRouter and failed.");
    }
}

AudioRouter::~AudioRouter() {
    for (uint8_t i = 0; i < 12; i++) free(inputs[i]);
    for (uint8_t i = 0; i < 8; i++)  free(outputs[i]);

    // Destroy the objects that represeent our hardware. Downstream operations will
    //   put the hardware into an inert state, so that needn't be done here.
	if (dp_lo != NULL)     delete dp_lo;
	if (dp_hi != NULL)     delete dp_hi;
	if (cp_switch != NULL) delete cp_switch;
}


/*
* Do all the bus-related init.
*/
int8_t AudioRouter::init(void) {
	int8_t result = dp_lo->init();
	if (result != 0) {
		printf("Failed to init() dp_lo (0x%02x) with cause (%d).", i2c_addr_dp_lo, result);
		return AUDIO_ROUTER_ERROR_BUS;
	}
	result = dp_hi->init();
	if (result != 0) {
		printf("Failed to init() dp_hi (0x%02x) with cause (%d).", i2c_addr_dp_hi, result);
		return AUDIO_ROUTER_ERROR_BUS;
	}
	result = cp_switch->init();
	if (result != 0) {
		printf("Failed to init() cp_switch (0x%02x) with cause (%d).", i2c_addr_cp_switch, result);
		return AUDIO_ROUTER_ERROR_BUS;
	}
	
	// If we are this far, it means we've successfully refreshed all the device classes
	//   to reflect the state of the hardware. Now to parse that data into structs that
	//   mean something to us at this level...
	for (int i = 0; i < 8; i++) {  // Volumes...
		outputs[i]->dp_val = outputs[i]->dp_dev->getValue(outputs[i]->dp_reg);
	}
	
	for (int i = 0; i < 12; i++) {  // Routes...
		uint8_t temp_byte = cp_switch->getValue(inputs[i]->cp_row);
		for (int j = 0; j < 8; j++) {
			if (0x01 & temp_byte) {
				CPOutputChannel* temp_output = getOutputByCol(j);
				if (temp_output != NULL) {
					temp_output->cp_row = inputs[i];
				}
			}
			temp_byte = temp_byte >> 1;
		}
	}
	
	return AUDIO_ROUTER_ERROR_NO_ERROR;
}


CPOutputChannel* AudioRouter::getOutputByCol(uint8_t col) {
	if (col > 7)  return NULL;
	for (int j = 0; j < 8; j++) {
		if (outputs[j]->cp_column == col) {
			return outputs[j];
		}
	}
	return NULL;
}


void AudioRouter::preserveOnDestroy(bool x) {
	if (dp_lo != NULL) dp_lo->preserveOnDestroy(x);
	if (dp_hi != NULL) dp_hi->preserveOnDestroy(x);
	if (cp_switch != NULL) cp_switch->preserveOnDestroy(x);
}


/*
* Leak warning: There is no call to free(). Nor should there be unless we can
*   be assured that a const will not be passed in. In which case, we are probably
*   wasting precious RAM.
*/
int8_t AudioRouter::nameInput(uint8_t row, const char* name) {
	if (row > 11) return AUDIO_ROUTER_ERROR_BAD_ROW;
	inputs[row]->name = (char *) name;
	return AUDIO_ROUTER_ERROR_NO_ERROR;
}


/*
* Leak warning: There is no call to free(). Nor should there be unless we can
*   be assured that a const will not be passed in. In which case, we are probably
*   wasting precious RAM.
*/
int8_t AudioRouter::nameOutput(uint8_t col, const char* name) {
	if (col > 7) return AUDIO_ROUTER_ERROR_BAD_COLUMN;
	outputs[col]->name = (char *) name;
	return AUDIO_ROUTER_ERROR_NO_ERROR;
}



int8_t AudioRouter::unroute(uint8_t col, uint8_t row) {
	if (col > 7)  return AUDIO_ROUTER_ERROR_BAD_COLUMN;
	if (row > 11) return AUDIO_ROUTER_ERROR_BAD_ROW;
	bool remove_link = (outputs[col]->cp_row == inputs[row]) ? true : false;
	uint8_t return_value = AUDIO_ROUTER_ERROR_NO_ERROR;
	if (cp_switch->unsetRoute(outputs[col]->cp_column, row) < 0) {
		return AUDIO_ROUTER_ERROR_UNROUTE_FAILED;
	}
	if (remove_link) outputs[col]->cp_row = NULL;
	return return_value;
}


int8_t AudioRouter::unroute(uint8_t col) {
	if (col > 7)  return AUDIO_ROUTER_ERROR_BAD_COLUMN;
	uint8_t return_value = AUDIO_ROUTER_ERROR_NO_ERROR;
	for (int i = 0; i < 12; i++) {
		if (unroute(col, i) != AUDIO_ROUTER_ERROR_NO_ERROR) {
			return AUDIO_ROUTER_ERROR_UNROUTE_FAILED;
		}
	}
	return return_value;
}


/*
* Remember: This is the class responsible for ensuring that we don't cross-wire a circuit.
*   The crosspoint switch class is generalized, and knows nothing about the circuit it is
*   embedded within. It will follow whatever instructions it is given. So if we tell it
*   to route two inputs to the same output, it will oblige and possibly fry hastily-built
*   hardware. So under that condition, we unroute prior to routing and return a code to
*   indicate that we've done so.
*/
int8_t AudioRouter::route(uint8_t col, uint8_t row) {
	uint8_t return_value = AUDIO_ROUTER_ERROR_NO_ERROR;
	if (col > 7)  return AUDIO_ROUTER_ERROR_BAD_COLUMN;
	if (row > 11) return AUDIO_ROUTER_ERROR_BAD_ROW;
	
	if (outputs[col]->cp_row != NULL) {
		int8_t result = unroute(col);
		if (result == AUDIO_ROUTER_ERROR_NO_ERROR) {
			return_value = AUDIO_ROUTER_ERROR_INPUT_DISPLACED;
			outputs[col]->cp_row = NULL;
		}
		else {
			return_value = AUDIO_ROUTER_ERROR_UNROUTE_FAILED;
		}
	}
	
	if (return_value >= 0) {
		int8_t result = cp_switch->setRoute(outputs[col]->cp_column, row);
		if (result != AUDIO_ROUTER_ERROR_NO_ERROR) {
			return_value = result;
		}
		else {
			outputs[col]->cp_row = inputs[row];
		}
	}
	
	return return_value;
}


int8_t AudioRouter::setVolume(uint8_t col, uint8_t vol) {
	int8_t return_value = AUDIO_ROUTER_ERROR_NO_ERROR;
	if (col > 7)  return AUDIO_ROUTER_ERROR_BAD_COLUMN;
	return_value = outputs[col]->dp_dev->setValue(outputs[col]->dp_reg, vol);
	return return_value;
}



// Turn on the chips responsible for routing signals.
int8_t AudioRouter::enable(void) {
	int8_t result = dp_lo->enable();
	if (result != 0) {
		printf("enable() failed to enable dp_lo. Cause: (%d).\n", result);
		return result;
	}
	result = dp_hi->enable();
	if (result != 0) {
		printf("enable() failed to enable dp_hi. Cause: (%d).\n", result);
		return result;
	}
	return AudioRouter::AUDIO_ROUTER_ERROR_NO_ERROR;
}

// Turn off the chips responsible for routing signals.
int8_t AudioRouter::disable(void) {
	int8_t result = dp_lo->disable();
	if (result != 0) {
		printf("disable() failed to disable dp_lo. Cause: (%d).\n", result);
		return result;
	}
	result = dp_hi->disable();
	if (result != 0) {
		printf("disable() failed to disable dp_hi. Cause: (%d).\n", result);
		return result;
	}
	result = cp_switch->reset();
	if (result != 0) {
		printf("disable() failed to reset cp_switch. Cause: (%d).\n", result);
		return result;
	}
	return AudioRouter::AUDIO_ROUTER_ERROR_NO_ERROR;
}


void AudioRouter::dumpOutputChannel(uint8_t chan) {
	if (chan > 7)  {
		printf("dumpOutputChannel() was passed an out-of-bounds id.\n");
		return;
	}
	printf("Output channel %d\n", chan);
	
	if (outputs[chan]->name != NULL) printf("%s\n", outputs[chan]->name);
	printf("Switch column %d\n", outputs[chan]->cp_column);
	if (outputs[chan]->dp_dev == NULL) {
		printf("Potentiometer is NULL\n");
	}
	else {
		uint8_t temp_int = (outputs[chan]->dp_dev == dp_lo) ? 0 : 1;
		printf("Potentiometer:            %d\n", temp_int);
		printf("Potentiometer register:   %d\n", outputs[chan]->dp_reg);
		printf("Potentiometer value:      %d\n", outputs[chan]->dp_val);
	}
	if (outputs[chan]->cp_row == NULL) {
		printf("Output channel %d is presently unbound.\n", chan);
	}
	else {
		printf("Output channel %d is presently bound to the following input...\n", chan);
		dumpInputChannel(outputs[chan]->cp_row);
	}
}


void AudioRouter::dumpInputChannel(CPInputChannel *chan) {
	if (chan == NULL) {
		printf("dumpInputChannel() was passed an out-of-bounds id.\n");
		return;
	}

	if (chan->name != NULL) printf("%s\n", chan->name);
	printf("Switch row: %d\n", chan->cp_row);
}

void AudioRouter::dumpInputChannel(uint8_t chan) {
	if (chan > 11) {
		printf("dumpInputChannel() was passed an out-of-bounds id.\n");
		return;
	}
	printf("Input channel %d\n", chan);
	if (inputs[chan]->name != NULL) printf("%s\n", inputs[chan]->name);
	printf("Switch row: %d\n", inputs[chan]->cp_row);
}


// Write some status about the routes to the provided char buffer.
int8_t AudioRouter::status(char* output) {
	for (int i = 0; i < 8; i++) {
		dumpOutputChannel(i);
	}
	printf("\n");
	
	dp_lo->dumpToLog();
	dp_hi->dumpToLog();
	cp_switch->dumpToLog();
	
	return AudioRouter::AUDIO_ROUTER_ERROR_NO_ERROR;
}

