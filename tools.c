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
 * File:   tools.c
 * Author: Karl Wintermann
 * 
 * Created on 30. September 2017, 11:08
 */

#include "tools.h"
#include "logging.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <math.h>

char converted_string[16] = { 0 };

size_t write_memory(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  memory_t *mem = (memory_t *)userp;
 
  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */ 
    lprintf(LOG_ERR, "WriteMemoryCallback() => Not enough memory!"); 
    return 0;
  }
 
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

char *byte_converter(int64_t bytes, int iec_prefix) {
    
    char *locale_save = setlocale(LC_ALL, NULL);
    char *env_lang = getenv("LANG");
    if(env_lang != NULL) {
        setlocale(LC_ALL, env_lang);
    }
    
    double divisors[8];
    divisors[0] = 1.0;
    divisors[1] = iec_prefix ? 1024.0 : 1000.0;
    divisors[2] = pow(divisors[1], 2);
    divisors[3] = pow(divisors[1], 3);
    divisors[4] = pow(divisors[1], 4);
    divisors[5] = pow(divisors[1], 5);
    divisors[6] = pow(divisors[1], 6);
    divisors[7] = pow(divisors[1], 7);
    divisors[8] = pow(divisors[1], 8);
    
    double bytes_d = (double)bytes;
    
    if(bytes_d >= divisors[6]) {
        snprintf(converted_string, sizeof(converted_string), "%0.2lf %s", bytes_d/divisors[6], iec_prefix ? "EiB" : "EB");
    } else if(bytes_d >= divisors[5]) {
        snprintf(converted_string, sizeof(converted_string), "%0.2lf %s", bytes_d/divisors[5], iec_prefix ? "PiB" : "PB");
    } else if(bytes_d >= divisors[4]) {
        snprintf(converted_string, sizeof(converted_string), "%0.2lf %s", bytes_d/divisors[4], iec_prefix ? "TiB" : "TB");
    } else if(bytes_d >= divisors[3]) {
        snprintf(converted_string, sizeof(converted_string), "%0.2lf %s", bytes_d/divisors[3], iec_prefix ? "GiB" : "GB");
    } else if(bytes_d >= divisors[2]) {
        snprintf(converted_string, sizeof(converted_string), "%0.2lf %s", bytes_d/divisors[2], iec_prefix ? "MiB" : "MB");
    } else if(bytes_d >= divisors[1]) {
        snprintf(converted_string, sizeof(converted_string), "%0.2lf %s", bytes_d/divisors[1], iec_prefix ? "KiB" : "kB");
    } else if(bytes_d >= divisors[0]) {
        snprintf(converted_string, sizeof(converted_string), "%ld %s", bytes, "B");
    }
    
    setlocale(LC_ALL, locale_save);
    return converted_string;
}