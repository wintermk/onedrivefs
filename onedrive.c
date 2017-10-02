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
 * File:   onedrive.c
 * Author: Karl Wintermann
 *
 * Created on 26. September 2017, 10:50
 */

#include "onedrive.h"
#include "config.h"
#include "auth.h"
#include "tools.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>


int onedrive_error(const char *method, struct json_object *answ_jobj) {
    struct json_object *err_jobj;
    if(json_object_object_get_ex(answ_jobj, "error", &err_jobj)) {       
        struct json_object *err_code_jobj, *err_message_jobj;
        if(json_object_object_get_ex(err_jobj, "code", &err_code_jobj) 
                && json_object_object_get_ex(err_jobj, "message", &err_message_jobj)) {
            lprintf(LOG_ERR, "%s => OneDrive Error\n"
                             "Error code: %s\n"
                             "Error message: %s",
                                method,
                                json_object_get_string(err_code_jobj),
                                json_object_get_string(err_message_jobj));
            
        } else {
            lprintf(LOG_ERR, "%s => OneDrive Error\n%s", method,
                    json_object_get_string(err_jobj));
        }
        return 1;
    }
    return 0;
}

void print_drives() {
    
    memory_t mem;
    mem.memory = malloc(1);
    mem.size = 0;
    
    if(mem.memory == NULL) {
        lprintf(LOG_ERR, "print_drives() => Out of memory!");
        return;
    }
    
    CURL *curl = curl_easy_init();
    if(curl == NULL) {
        lprintf(LOG_ERR, "print_drives() => Can't initialize curl!");
        return;
    }
       
    struct curl_slist *list = NULL;
    list = append_access_token(list);
    
    if(list == NULL) {
        curl_easy_cleanup(curl);
        return;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, API_URI DRIVES);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&mem);
    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(list);
    curl_easy_cleanup(curl);
    
    if(res != CURLE_OK) { 
        lprintf(LOG_ERR, "print_drives() => Curl perfom error %d\n", res);
        return;
    }
    
    lprintf(LOG_DEBUG, "print_drives() => JSON string: %s", mem.memory);
    
    enum json_tokener_error jerr;
    struct json_object *answ_jobj;
    
    answ_jobj = json_tokener_parse_verbose(mem.memory, &jerr);
    free(mem.memory);
    
    if(answ_jobj == NULL) {
        lprintf(LOG_ERR, "print_drives() => JSON parse error!\n"
                         "Error description: %s\n", json_tokener_error_desc(jerr));
        return;
    }
    
    if(onedrive_error("print_drives()", answ_jobj)) {
        return;
    }
    
    struct json_object *value_jobj, *array_value, *id_jobj, *driveType_jobj, 
            *owner_jobj, *user_jobj, *displayName_jobj, *userId_jobj,
            *quota_jobj, *deleted_jobj, *remaining_jobj, *state_jobj, *total_jobj, *used_jobj;
    
    if(json_object_object_get_ex(answ_jobj, "value", &value_jobj)) {
        for(int i=0; i<json_object_array_length(value_jobj); i++) {
            array_value = json_object_array_get_idx(value_jobj, i);
            if(array_value != NULL) {
                printf("Drive %d:\n", i+1);
                if(json_object_object_get_ex(array_value, "id", &id_jobj)) {
                    printf("  Drive id: %s\n", json_object_get_string(id_jobj));
                }
                if(json_object_object_get_ex(array_value, "driveType", &driveType_jobj)) {
                    printf("  Drive type: %s\n", json_object_get_string(driveType_jobj));
                }
                if(json_object_object_get_ex(array_value, "owner", &owner_jobj)) {
                    if(json_object_object_get_ex(owner_jobj, "user", &user_jobj)) {
                        printf("  Owner:\n");
                        if(json_object_object_get_ex(user_jobj, "displayName", &displayName_jobj)) {
                            printf("    User: %s\n", json_object_get_string(displayName_jobj));
                        }
                        if(json_object_object_get_ex(user_jobj, "id", &userId_jobj)) {
                            printf("    User id: %s\n", json_object_get_string(userId_jobj));
                        }
                    }
                }
                if(json_object_object_get_ex(array_value, "quota", &quota_jobj)) {
                    printf("  Quota:\n");
                    if(json_object_object_get_ex(quota_jobj, "state", &state_jobj)) {
                        printf("    State: %s\n", json_object_get_string(state_jobj));
                    }
                    if(json_object_object_get_ex(quota_jobj, "total", &total_jobj)) {
                        int64_t total = json_object_get_int64(total_jobj);
                        printf("    Total: %s (%ld B)\n", byte_converter(total, UNIT_PREFIX), total);
                    }
                    if(json_object_object_get_ex(quota_jobj, "used", &used_jobj)) {
                        int64_t used = json_object_get_int64(used_jobj);
                        printf("    Used: %s (%ld B)\n", byte_converter(used, UNIT_PREFIX), used);
                    }
                    if(json_object_object_get_ex(quota_jobj, "remaining", &remaining_jobj)) {
                        int64_t remaining = json_object_get_int64(remaining_jobj);
                        printf("    Free: %s (%ld B)\n", byte_converter(remaining, UNIT_PREFIX), remaining);
                    }
                    if(json_object_object_get_ex(quota_jobj, "deleted", &deleted_jobj)) {
                        int64_t deleted = json_object_get_int64(deleted_jobj);
                        printf("    Deleted: %s (%ld B)\n", byte_converter(deleted, UNIT_PREFIX), deleted);
                    }
                }
                printf("\n");
            }  
        }
        
    } else {
        printf("No drives were found!\n");
    } 
}
