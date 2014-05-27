/*
File:   Logger.h
Author: J. Ian Lindsay
Date:   2014.04.28


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


This class is designed to unify logging support on host and micro builds
  of other packages.
*/


#ifndef IANS_LOGGER_CLASS_H__
#define IANS_LOGGER_CLASS_H__

#include <stdarg.h>
#include <inttypes.h>

#ifndef ARDUINO
  #include <stdlib.h>
  #include <stdio.h>
  #include <syslog.h>
  #include <time.h>
  #include <string.h>
#else
  #include "Arduino.h"
  #define LOG_EMERG   0    /* system is unusable */
  #define LOG_ALERT   1    /* action must be taken immediately */
  #define LOG_CRIT    2    /* critical conditions */
  #define LOG_ERR     3    /* error conditions */
  #define LOG_WARNING 4    /* warning conditions */
  #define LOG_NOTICE  5    /* normal but significant condition */
  #define LOG_INFO    6    /* informational */
  #define LOG_DEBUG   7    /* debug-level messages */
#endif



class IansLogger {
  public:
	IansLogger(bool log_to_syslog, bool log_to_stdout);
	IansLogger(void);
	~IansLogger(void);
	
	void setVerbosity(uint8_t);

    void unified_log(const char *fxn_name, int severity, const char *str, ...);
    void unified_log(int severity, const char *str);
    void unified_log(const char *str);

  private:
    bool log_to_syslog;
    bool log_to_stdout;
    uint8_t verbosity;
    uint32_t supressed_log_count;
};


#endif
