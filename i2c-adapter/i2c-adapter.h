/*
File:   i2c-adapter.h
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


This class is supposed to be an i2c abstraction layer. The goal is to have
an object of this class that can be instantiated and used to communicate 
with i2c devices (as a bus master) regardless of the platform.

Prime targets for this layer are the linux operating system and the Teensy3,
but adding support for other platforms ought to be easy.
*/


#ifndef I2C_ABSTRACTION_LAYER         // This is meant to prevent double-inclusion.
  #define I2C_ABSTRACTION_LAYER 1

  #ifndef ARDUINO
    #include <sys/types.h>
    #include <stdint.h>
    #include <inttypes.h>
    #include <ctype.h>
    #include <fstream>
    #include <iostream>
    #include <sys/stat.h>
    #include <fcntl.h>
  #endif


  class I2CAdapter {

    public:
      bool bus_error;
        
#ifndef ARDUINO
      I2CAdapter(uint8_t);         // Constructor takes a bus ID as an argument. Useful on platforms that have several busses.
#else
      I2CAdapter(void);            // Constructor takes an optional device ID (bus ID) as an argument.
#endif
      ~I2CAdapter(void);           // Destructor

      bool busIdle(void);          // Returns true if the bus is ready to service a transaction right now. 
      bool busOnline(void);
      
      // Writes <byte_count> bytes from <buf> to the sub-address <sub_addr> of i2c device <dev_addr>.
      // Returns the number of bytes so written.
      int writeX(uint8_t dev_addr, uint8_t sub_addr, uint16_t byte_count, uint8_t *buf);
      int write8(uint8_t dev_addr, uint8_t dat);
      int write16(uint8_t dev_addr, uint16_t dat);
      int write8(uint8_t dev_addr, uint8_t sub_addr, uint8_t dat);
      int write16(uint8_t dev_addr, uint8_t sub_addr, uint16_t dat);

      // Convenience functions for reading bytes from a given i2c address/sub-address...
      uint8_t read8(uint8_t dev_addr);
      uint8_t read8(uint8_t dev_addr, uint8_t sub_addr);
      uint16_t read16(uint8_t dev_addr, uint8_t sub_addr);
      uint16_t read16(uint8_t dev_addr);
      uint16_t read16(uint8_t dev_addr, uint16_t sub_addr);
      int readX(uint8_t dev_addr, uint8_t sub_addr, uint8_t len, uint8_t *buf);
      
      void setDebug(bool);


    private:
      bool bus_online;
      bool bus_in_use;
      bool debug;
      
      uint8_t last_used_bus_addr;
#ifndef ARDUINO
      int open_bus_descriptor;
#endif

      bool switch_device(uint8_t);      // Call this to switch to another i2c device on the bus.
  };

#endif

