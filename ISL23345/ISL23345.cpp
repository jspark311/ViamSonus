/*
File:   ISL23345.cpp
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

#include "ISL23345.h"
const int8_t ISL23345::ISL23345_ERROR_DEVICE_DISABLED  = 3;    // A caller tried to set a wiper while the device is disabled. This may work...
const int8_t ISL23345::ISL23345_ERROR_PEGGED_MAX       = 2;    // There was no error, but a call to change a wiper setting pegged the wiper at its highest position.
const int8_t ISL23345::ISL23345_ERROR_PEGGED_MIN       = 1;    // There was no error, but a call to change a wiper setting pegged the wiper at its lowest position.
const int8_t ISL23345::ISL23345_ERROR_NO_ERROR         = 0;    // There was no error.
const int8_t ISL23345::ISL23345_ERROR_ABSENT           = -1;   // The ISL23345 appears to not be connected to the bus.
const int8_t ISL23345::ISL23345_ERROR_BUS              = -2;   // Something went wrong with the i2c bus.
const int8_t ISL23345::ISL23345_ERROR_ALREADY_AT_MAX   = -3;   // A caller tried to increase the value of the wiper beyond its maximum.  
const int8_t ISL23345::ISL23345_ERROR_ALREADY_AT_MIN   = -4;   // A caller tried to decrease the value of the wiper below its minimum.
const int8_t ISL23345::ISL23345_ERROR_INVALID_POT      = -5;   // The ISL23345 only has 4 potentiometers. 


#include "../Logger/Logger.h"
extern IansLogger logger;

/*
* This is an i2c-wrapper class. It is meant to abstract away the implementation
*   details of an i2c-supporting platform so that i2c-dependant classes don't
*   need re-work. If your platform is not supported by the abstraction layer,
*   you can simply replace the two lines below with lines appropriate for your
*   platform, or (preferably) you can extend i2c-adapter with the support you need.
* If you DO extend i2c-adapter, and you feel like improving the class for others,
*   submit your changes to josh.lindsay@gmail.com, and I will integrate your changes
*   into the code base.
*/
#include "../i2c-adapter/i2c-adapter.h"
extern I2CAdapter *i2c;



/*
* Constructor. Takes the i2c address of this device as sole argument.
*/
ISL23345::ISL23345(uint8_t i2c_addr) {
	I2C_ADDRESS = i2c_addr;
	dev_enabled = false;
	dev_init    = false;
	preserve_state_on_destroy = false;
	init();
}

/*
* When we destroy the class instance, the hardware will be disabled.
*/
ISL23345::~ISL23345(void) {
	if (!preserve_state_on_destroy) {
		disable();
	}
}


/*
* Call to read the device and cause this class's state to reflect that of the device. 
*/
int8_t ISL23345::init(void) {
	if ((i2c == NULL) || (!i2c->busOnline())) {
		dev_init = false;
		return ISL23345::ISL23345_ERROR_BUS;
	}
	int8_t return_value = ISL23345::ISL23345_ERROR_NO_ERROR;

	uint8_t result = i2c->read8(I2C_ADDRESS, 0x10);
	if (!i2c->bus_error) {
		// If no error, we take the read value to accurately reflect our enable-state.
		dev_enabled = ((result & 0x40) > 0);
		dev_init = true;
	}
	else {
		return_value = ISL23345_ERROR_ABSENT;
		dev_init = false;
	}
	
	if (dev_init) {
		for (uint8_t i = 0; i < 4; i++) {
			result = i2c->read8(I2C_ADDRESS, i);
			if (!i2c->bus_error) {
				values[i] = result;
			}
			else {
				dev_init = false;
				return ISL23345_ERROR_ABSENT;
			}
		}
	}
	
	return return_value;
}



void ISL23345::preserveOnDestroy(bool x) {
	preserve_state_on_destroy = x;
}



/*
* Enable the device. Reconnects Rh pins and restores the wiper settings.
*/
int8_t ISL23345::enable() {
	if (!i2c->busOnline()) {
		return ISL23345::ISL23345_ERROR_BUS;
	}
	int8_t return_value = ISL23345::ISL23345_ERROR_NO_ERROR;
	if (i2c->write8(I2C_ADDRESS, 0x10, 0x40) > 0) {
		dev_enabled = true;
	}
	else {
		return_value = ISL23345::ISL23345_ERROR_ABSENT;
	}
	return return_value;
}


/*
* Disable the device. Disconnects all Rh pins and sets the wiper to 2k ohm WRT Rl.
* Retains wiper settings.
*/
int8_t ISL23345::disable() {
	if (!i2c->busOnline()) {
		return ISL23345::ISL23345_ERROR_BUS;
	}
	int8_t return_value = ISL23345::ISL23345_ERROR_NO_ERROR;
	if (i2c->write8(I2C_ADDRESS, 0x10, 0x00) > 0) {
		dev_enabled = false;
	}
	else {
		return_value = ISL23345::ISL23345_ERROR_ABSENT;
	}
	return return_value;
}


/*
* Set the value of the given wiper to the given value.
*/
int8_t ISL23345::setValue(uint8_t pot, uint8_t val) {
	if (pot > 3)    return ISL23345::ISL23345_ERROR_INVALID_POT;
	if (!dev_init)  return ISL23345::ISL23345_ERROR_DEVICE_DISABLED;
	if (!i2c->busOnline()) {
		return ISL23345::ISL23345_ERROR_BUS;
	}

	int8_t return_value = ISL23345::ISL23345_ERROR_NO_ERROR;
	if (i2c->write8(I2C_ADDRESS, pot, val) <= 0) {
		return_value = ISL23345::ISL23345_ERROR_ABSENT;
	}
	values[pot] = val;
	return return_value;
}



uint8_t ISL23345::getValue(uint8_t pot) {
	if (pot > 3) return ISL23345_ERROR_INVALID_POT;
	return values[pot];
}


/*
* Calling this function will take the device out of shutdown mode and set all the wipers
*   to their minimum values.
*/
int8_t ISL23345::reset(void) {
	return reset(0x00);
}


int8_t ISL23345::reset(uint8_t val) {
	int8_t result = 0;
	for (int i = 0; i < 4; i++) {
		result = setValue(0, val);
		if (result != ISL23345_ERROR_NO_ERROR) {
			return result;
		}
	}
	return ISL23345_ERROR_NO_ERROR;
}


bool     ISL23345::enabled(void) {     return dev_enabled;  }  // Trivial accessor.
uint16_t ISL23345::getRange(void) {    return 0x00FF;       }  // Trivial. Returns the maximum vaule of any single potentiometer.


void ISL23345::dumpToLog(void) {
	logger.unified_log(__PRETTY_FUNCTION__, LOG_INFO, "Device i2c address is 0x%02x", I2C_ADDRESS);

	if (!dev_init) {
		logger.unified_log(__PRETTY_FUNCTION__, LOG_INFO, "0x%02x is not initialized.", I2C_ADDRESS);
		return;
	}
	
	logger.unified_log(__PRETTY_FUNCTION__, LOG_INFO, "0x%02x is%s enabled.", I2C_ADDRESS, ((dev_enabled) ? "" : " not"));
	for (int i = 0; i < 4; i++) {
		logger.unified_log(__PRETTY_FUNCTION__, LOG_INFO, "  POT %d: 0x%02x", i, values[i]);
	}
	
}

