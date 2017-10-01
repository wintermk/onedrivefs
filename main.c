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
 * File:   main.c
 * Author: Karl Wintermann
 *
 * Created on 16. September 2017, 21:02
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "auth.h"
#include "config.h"
#include "logging.h"
#include "onedrive.h"



/*
 * 
 */
int main(int argc, char** argv) {


    curl_global_init(CURL_GLOBAL_DEFAULT);
    auth_init("/home/karl/.OneDriveFS");
    
    print_drives();
    
    auth_cleanup();
    curl_global_cleanup();
    return (EXIT_SUCCESS);
}

