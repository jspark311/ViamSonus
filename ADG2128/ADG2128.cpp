/*
File:   ADG2128.cpp
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

#include "ADG2128.h"

const int8_t ADG2128::ADG2128_ERROR_NO_ERROR         = 0; 
const int8_t ADG2128::ADG2128_ERROR_ABSENT           = -1;
const int8_t ADG2128::ADG2128_ERROR_BUS              = -2;
const int8_t ADG2128::ADG2128_ERROR_BAD_COLUMN       = -3;   // Column was out-of-bounds.
const int8_t ADG2128::ADG2128_ERROR_BAD_ROW          = -4;   // Row was out-of-bounds.


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
ADG2128::ADG2128(uint8_t i2c_addr) {
	I2C_ADDRESS = i2c_addr;
	preserve_state_on_destroy = false;
	dev_init = false;
	init();
}

ADG2128::~ADG2128(void) {
	if (!preserve_state_on_destroy) {
		reset();
	}
}


/*
* 
*/
int8_t ADG2128::init(void) {
	if ((i2c == NULL) || (!i2c->busOnline())) {
		logger.unified_log(__PRETTY_FUNCTION__, LOG_ERR, "Bus not ready.");
		dev_init = false;
		return ADG2128_ERROR_BUS;
	}
	for (int i = 0; i < 12; i++) {
		if (readback(i) != ADG2128_ERROR_NO_ERROR) {
			dev_init = false;
			logger.unified_log(__PRETTY_FUNCTION__, LOG_ERR, "Failed to init switch.");
			return ADG2128_ERROR_BUS;
		}
	}
	dev_init = true;
	return ADG2128_ERROR_NO_ERROR;
}


    
int8_t ADG2128::setRoute(uint8_t col, uint8_t row) {
	if (col > 7)  return ADG2128_ERROR_BAD_COLUMN;
	if (row > 11) return ADG2128_ERROR_BAD_ROW;
	if ((i2c == NULL) || (!i2c->busOnline())) {
		logger.unified_log(__PRETTY_FUNCTION__, LOG_ERR, "Bus not ready.");
		return ADG2128_ERROR_BUS;
	}
	uint8_t safe_row = row;
	if (safe_row >= 6) safe_row = safe_row + 2;
	uint16_t val = 0x01 + ((0x80 + (safe_row << 3) + col) << 8);
	if (i2c->write16(I2C_ADDRESS, val) <= 0) {
		logger.unified_log(__PRETTY_FUNCTION__, LOG_ERR, "Failed to write new value.");
		return ADG2128_ERROR_BUS;
	}
	values[row] = values[row] | (0x01 << col);
	return ADG2128_ERROR_NO_ERROR;
}


int8_t ADG2128::unsetRoute(uint8_t col, uint8_t row) {
	if (col > 7)  return ADG2128_ERROR_BAD_COLUMN;
	if (row > 11) return ADG2128_ERROR_BAD_ROW;
	if ((i2c == NULL) || (!i2c->busOnline())) {
		logger.unified_log(__PRETTY_FUNCTION__, LOG_ERR, "Bus not ready.");
		return ADG2128_ERROR_BUS;
	}
	uint8_t safe_row = row;
	if (safe_row >= 6) safe_row = safe_row + 2;
	uint16_t val = 0x01 + ((0x00 + (safe_row << 3) + col) << 8);
	if (i2c->write16(I2C_ADDRESS, val) <= 0) {
		logger.unified_log(__PRETTY_FUNCTION__, LOG_ERR, "Failed to write new value.");
		return ADG2128_ERROR_BUS;
	}
	values[row] = values[row] & ~(0x01 << col);
	return ADG2128_ERROR_NO_ERROR;
}


void ADG2128::preserveOnDestroy(bool x) {
	preserve_state_on_destroy = x;
}


/*
* Opens all switches.
*/
int8_t ADG2128::reset(void) {
	for (int i = 0; i < 12; i++) {
		for (int j = 0; j < 8; j++) {
			if (unsetRoute(j, i) != ADG2128_ERROR_NO_ERROR) {
				return ADG2128_ERROR_BUS;
			}
		}
	}
	return init();
}


/*
* Readback on this part is organized by rows, with the return bits
* being the state of the switches to the ocrresponding column.
* The readback address table is hard-coded in the readback_addr array.
*
*
*/
int8_t ADG2128::readback(uint8_t row) {
	if (row > 11) return ADG2128_ERROR_BAD_ROW;
	if ((i2c == NULL) || (!i2c->busOnline())) {
		logger.unified_log(__PRETTY_FUNCTION__, LOG_ERR, "Bus not ready.");
		return ADG2128_ERROR_BUS;
	}
	uint16_t readback_addr[12] = {0x3400, 0x3b00, 0x7400, 0x7b00, 0x3500, 0x3D00, 0x7500, 0x7D00, 0x3600, 0x3E00, 0x7600, 0x7E00};
	//i2c->write16(I2C_ADDRESS, readback_addr[row]);
	uint16_t val = 0;
	val = i2c->read16(I2C_ADDRESS, readback_addr[row]);
	if (!i2c->bus_error) {
		values[row] = (uint8_t) val;
	}
	else {
		logger.unified_log(__PRETTY_FUNCTION__, LOG_ERR, "Bus error while reading readback address %d.", row);
		return ADG2128_ERROR_ABSENT;
	}
	return ADG2128_ERROR_NO_ERROR;
}


uint8_t ADG2128::getValue(uint8_t row) {
	if (row > 11) return ADG2128_ERROR_BAD_ROW;
	readback(row);
	return values[row];
}


void ADG2128::dumpToLog(void) {
	logger.unified_log(__PRETTY_FUNCTION__, LOG_INFO, "Device i2c address is 0x%02x", I2C_ADDRESS);
	if (dev_init) {
		for (int i = 0; i < 12; i++) {
			logger.unified_log(__PRETTY_FUNCTION__, LOG_INFO, "Row %d: %d", i, (uint8_t) getValue(i));
		}
	}
	else {
		logger.unified_log(__PRETTY_FUNCTION__, LOG_INFO, "Device 0x%02x is not initialized.", I2C_ADDRESS);
	}
}

