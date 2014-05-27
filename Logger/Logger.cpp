/*
File:   Logger.cpp
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

#include "Logger.h"


IansLogger::IansLogger(bool log_2_syslog, bool log2_stdout) {
	log_to_syslog = log_2_syslog;
	log_to_stdout = log2_stdout;
	supressed_log_count = 0;
	verbosity = 5;
}

IansLogger::IansLogger() {
	log_to_syslog = false;
	log_to_stdout = true;
	supressed_log_count = 0;
	verbosity = 5;
}

IansLogger::~IansLogger() {
}


void IansLogger::setVerbosity(uint8_t nu_verbosity) {
	verbosity = (nu_verbosity > 7) ? (nu_verbosity % 8) : nu_verbosity;
}


// Log a message. Target is determined by the current_config.
//    If no logging target is specified, log to stdout.
void IansLogger::unified_log(const char *fxn_name, int severity, const char *str, ...) {
	if (severity > verbosity) {
		supressed_log_count++;
		return;
	}
    int len = strlen(str);
    unsigned short f_codes = 0;  // Count how many format codes are in use...
    for (unsigned short i = 0; i < len; i++) {  if (*(str+i) == '%') f_codes++; }
    
    uint16_t total_projected_len = strlen(fxn_name) + 1 + len + (f_codes * 15);
    char *log_arg    = (char *) alloca(total_projected_len);

    va_list marker;
    va_start(marker, str);
    int ret = vsprintf(log_arg, str, marker);
    va_end(marker);
    if (ret < 0) {
      log_arg = (char *) "FAILED TO ALLOCATE MEMORY FOR LOG LINE\n";
    }
#ifndef ARDUINO
    int log_disseminated    = 0;
    time_t seconds = time(NULL);
    char *time_str    = (char *) alloca(32);
    strftime(time_str, 32, "%c", gmtime(&seconds));
    if (log_to_syslog) {
        syslog(severity, "%s", log_arg);
        log_disseminated    = 1;
    }

    if ((log_disseminated != 1) || log_to_stdout){
        printf("%s  %s:    %s\n", time_str, fxn_name, log_arg);        // Log to stdout.
    }
#else
    Serial.print(fxn_name);
    Serial.print("  ");
    Serial.print(String(severity, 10));
    Serial.print("  ");
    Serial.println(log_arg);
#endif
}



void IansLogger::unified_log(int severity, const char *str) {
	if (severity > verbosity) {
		supressed_log_count++;
		return;
	}
#ifndef ARDUINO
    time_t seconds = time(NULL);
    char *time_str    = (char *) alloca(32);
    strftime(time_str, 32, "%c", gmtime(&seconds));
    printf("%s:    %s\n", time_str, str);        // Log to stdout.
#else
    Serial.print(String(severity, 10));
    Serial.print("  ");
    Serial.println(str);
#endif
}


// Trivial override function...
void IansLogger::unified_log(const char *str) {
	unified_log(0, str);
}


IansLogger logger;
