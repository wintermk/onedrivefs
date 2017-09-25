/*
 * Copyright (C) 2017 Karl Wintermann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * File:   logging.c
 * Author: Karl Wintermann
 * 
 * Created on 23. September 2017, 10:55
 */

#include "logging.h"
#include "config.h"


void lprintf(int level, const char *format, ...) {
   
#ifndef DEBUG
    if(level > LOG_WARNING)
        return;
    openlog(PNAME, LOG_CONS, LOG_USER);
#else
    openlog(PNAME, LOG_CONS | LOG_PERROR, LOG_USER);
#endif
    va_list arg_ptr;
    va_start(arg_ptr, format);
    vsyslog(level, format, arg_ptr);
    va_end(arg_ptr);
    closelog();
}
