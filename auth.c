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
 * File:   auth.c
 * Author: Karl Wintermann
 * 
 * Created on 17. September 2017, 12:23
 */

#include "auth.h"
#include "config.h"
#include "logging.h"
#include "tools.h"
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <json-c/json.h>

typedef struct token_s {
    char *token_type_str;
    char *scope_str;
    char *redirect_uri_str;
    char *access_token_str;
    char *refresh_token_str;
    time_t created;
    int64_t expires_in;   
} token_t;

char tokenFile[PATH_MAX] = { 0 };

token_t token;

int token_initialized() {
    if( (token.token_type_str == NULL) &&
        (token.scope_str == NULL) &&
        (token.redirect_uri_str == NULL) &&
        (token.access_token_str == NULL) &&
        (token.refresh_token_str == NULL) &&
        (token.created == 0) &&
        (token.expires_in == 0) ) {
        return 1;
    }
    return 0;
}

void init_token() {
    token.token_type_str = NULL;
    token.scope_str = NULL;
    token.redirect_uri_str = NULL;
    token.access_token_str = NULL;
    token.refresh_token_str = NULL;  
    token.created = 0;
    token.expires_in = 0;
}

void free_token() {
    free(token.token_type_str);
    free(token.scope_str);
    free(token.redirect_uri_str);
    free(token.access_token_str);
    free(token.refresh_token_str);
}

void auth_init(char *token_dir) {
    memset(tokenFile, 0, sizeof(tokenFile));
    strncpy(tokenFile, token_dir, PATH_MAX-11); // dir + "/token.dat" + '\0' <= PATH_MAX
    strncat(tokenFile, "/token.dat", 10);
    lprintf(LOG_DEBUG, "auth_init() => tokenFile = %s", tokenFile);
    init_token();   
}

void auth_cleanup() {
    free_token();
    init_token();
}

int save_token() {
    FILE *fp;
    
    if(token_initialized() || strlen(tokenFile) == 0) {
        lprintf(LOG_ERR, "save_token() => empty token!");
        return 0;
    }
    
    fp = fopen(tokenFile, "w+");
    if(fp == NULL) {
        lprintf(LOG_ERR, "save_token() => Can't open token file for writing!");
        return 0;
    }
     
    fprintf(fp, "%s\n%s\n%s\n%s\n%s\n%ld\n%ld\n",
            token.token_type_str, token.scope_str, token.redirect_uri_str, 
            token.access_token_str, token.refresh_token_str,
            token.created, token.expires_in);
    
    fclose(fp);
    return 1;
}

int load_token() {
    FILE *fp;
    size_t len = 0;
    if(strlen(tokenFile) == 0) {
        lprintf(LOG_ERR, "load_token() => tokenFile empty!");
        return 0;
    }
    
    free_token();
    
    fp = fopen(tokenFile, "r");
    if(fp == NULL) {
        lprintf(LOG_ERR, "load_token() => Can't open token file for reading!");
        return 0;
    }
    if(getline(&token.token_type_str, &len, fp) <= 0) {
        lprintf(LOG_ERR, "load_token() => Can't read token type!");
        fclose(fp);
        return 0;
    }
    len = 0;
    if(getline(&token.scope_str, &len, fp) <= 0) {
        lprintf(LOG_ERR, "load_token() => Can't read scope!");
        fclose(fp);
        return 0;
    }
    len = 0;
    if(getline(&token.redirect_uri_str, &len, fp) <= 0) {
        lprintf(LOG_ERR, "load_token() => Can't read redirect uri!");
        fclose(fp);
        return 0;
    }
    len = 0;
    if(getline(&token.access_token_str, &len, fp) <= 0) {
        lprintf(LOG_ERR, "load_token() => Can't read access token!");
        fclose(fp);
        return 0;
    }
    len = 0;
    if(getline(&token.refresh_token_str, &len, fp) <= 0) {
        lprintf(LOG_ERR, "load_token() => Can't read refresh token!");
        fclose(fp);
        return 0;
    }
    if(fscanf(fp, "%ld\n%ld\n", &token.created, &token.expires_in) < 2) {
        lprintf(LOG_ERR, "load_token() => Can't read token time values!");
        fclose(fp);
        return 0;
    }
    
    token.token_type_str[strlen(token.token_type_str)-1] = 0;
    token.scope_str[strlen(token.scope_str)-1] = 0;
    token.redirect_uri_str[strlen(token.redirect_uri_str)-1] = 0;
    token.access_token_str[strlen(token.access_token_str)-1] = 0;
    token.refresh_token_str[strlen(token.refresh_token_str)-1] = 0;
    
    fclose(fp);
    return 1;
}

int auth(char *redirect_uri_str) {
    
    memory_t mem;
    mem.memory = malloc(1);
    mem.size = 0;
    
    if(mem.memory == NULL) {
        lprintf(LOG_ERR, "auth() => Out of memory!");
        return 0;
    }
    
    if(redirect_uri_str == NULL) {
        lprintf(LOG_ERR, "auth() =>  Redirect uri pointer is NULL!");
        return 0;
    }
    
    free_token();
    
    char *uri = strtok(redirect_uri_str, "?");
    char *args = strtok(NULL, "?");
    
    CURL *curl = curl_easy_init();
    if(curl == NULL) {
        lprintf(LOG_ERR, "auth() => Can't initialize curl!");
        return 0;
    }
    char *uri_encoded = curl_easy_escape(curl, uri, strlen(uri));
    if(uri_encoded == NULL) {
        lprintf(LOG_ERR, "auth() => Can't escape redirect uri!");
        curl_easy_cleanup(curl);
        return 0;
    }
    
    token.redirect_uri_str = strdup(uri_encoded);
    if(token.redirect_uri_str == NULL) {
        lprintf(LOG_ERR, "auth() => strdup: Out of memory!");
        curl_free(uri_encoded);
        curl_easy_cleanup(curl);
        return 0;
    }
    curl_free(uri_encoded);
    
    char *ptr = strtok(args, "&");
    char *code_arg = NULL;
    while(ptr != NULL) {
        if(!strncmp(ptr, "code=", 5)) {
            code_arg = ptr;
            break;
        }
        ptr = strtok(NULL, "&");
    }
    if(code_arg == NULL) {
        lprintf(LOG_ERR, "auth() => Redirect uri does not contain an code parameter!");
        curl_easy_cleanup(curl);
        return 0;
    }
    
    strtok(code_arg, "=");
    char *code = strtok(NULL, "=");
    if(code == NULL) {
        lprintf(LOG_ERR, "auth() => Empty auth code!");
        curl_easy_cleanup(curl);
        return 0;
    }
    
    size_t postfields_len = (strlen("client_id=" CLIENT_ID) + 
                             strlen("&scope=" SCOPE) + 
                             strlen("&code=") + strlen(code) +
                             strlen("&redirect_uri=") + 
                                strlen(token.redirect_uri_str) +
                             strlen("&grant_type=" GRANT_TYPE_AUTH) + 1 
                            ) * sizeof(char);
    
    char *postfields = malloc(postfields_len);
    if(postfields == NULL) {
        lprintf(LOG_ERR, "auth() => Post fields malloc error!");
        curl_easy_cleanup(curl);
        return 0;
    }
    
    postfields[0] = 0;
    
    strcat(postfields, "client_id=" CLIENT_ID);
    strcat(postfields, "&scope=" SCOPE);
    strcat(postfields, "&code=");
    strcat(postfields, code);
    strcat(postfields, "&redirect_uri=");
    strcat(postfields, token.redirect_uri_str);
    strcat(postfields, "&grant_type=" GRANT_TYPE_AUTH);
        
    token.created = time(NULL);
    
    curl_easy_setopt(curl, CURLOPT_URL, TOKEN_URI);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&mem);
    CURLcode res = curl_easy_perform(curl);
    free(postfields);
    if(res != CURLE_OK) {
        lprintf(LOG_ERR, "auth() => Curl perfom error %d\n", res);
        curl_easy_cleanup(curl);
        return 0;   
    }
    curl_easy_cleanup(curl);
    
    lprintf(LOG_DEBUG, "auth() => JSON string: %s", mem.memory);
    
    enum json_tokener_error jerr;
    struct json_object *answ_jobj, *err_jobj;
    
    answ_jobj = json_tokener_parse_verbose(mem.memory, &jerr);
    free(mem.memory);
    
    if(answ_jobj == NULL) {
        lprintf(LOG_ERR, "auth() => JSON parse error!\n"
                         "Error description: %s\n", json_tokener_error_desc(jerr));
        return 0;
    }
    
    if(json_object_object_get_ex(answ_jobj, "error", &err_jobj)) {
        lprintf(LOG_ERR, "auth() => Auth error: %s", json_object_get_string(err_jobj));
        struct json_object *err_desc_jobj;
        if(json_object_object_get_ex(answ_jobj, "error_description", &err_desc_jobj)) {
            lprintf(LOG_ERR, "Error description: %s", json_object_get_string(err_desc_jobj));
        }
        return 0;
    }
    
    struct json_object *token_type_jobj, *scope_jobj, *expires_in_jobj,
            *access_token_jobj, *refresh_token_jobj;
    
    if(json_object_object_get_ex(answ_jobj, "token_type", &token_type_jobj)) {
        token.token_type_str = strdup(json_object_get_string(token_type_jobj));
        if(token.token_type_str == NULL) {
            lprintf(LOG_ERR, "auth() => strdup: Out of memory!");
            return 0;
        }
    } else {
        lprintf(LOG_ERR, "auth() => Answer does not contain \"token_type\" value!");
        return 0;
    }
    
    if(json_object_object_get_ex(answ_jobj, "scope", &scope_jobj)) {
        token.scope_str = strdup(json_object_get_string(scope_jobj));
        if(token.scope_str == NULL) {
            lprintf(LOG_ERR, "auth() => strdup: Out of memory!");
            return 0;
        }
    } else {
        lprintf(LOG_ERR, "auth() => Answer does not contain \"scope\" value!");
        return 0;
    }
    
    if(json_object_object_get_ex(answ_jobj, "expires_in", &expires_in_jobj)) {
        token.expires_in = json_object_get_int64(expires_in_jobj);
    } else {
        lprintf(LOG_ERR, "auth() => Answer does not contain \"expires_in\" value!");
        return 0;
    }
    
    if(json_object_object_get_ex(answ_jobj, "access_token", &access_token_jobj)) {
        token.access_token_str = strdup(json_object_get_string(access_token_jobj));
        if(token.access_token_str == NULL) {
            lprintf(LOG_ERR, "auth() => strdup: Out of memory!");
            return 0;
        }
    } else {
        lprintf(LOG_ERR, "auth() => Answer does not \"access_token\" value!");
        return 0;
    }
    
    if(json_object_object_get_ex(answ_jobj, "refresh_token", &refresh_token_jobj)) {
        token.refresh_token_str = strdup(json_object_get_string(refresh_token_jobj));
        if(token.refresh_token_str == NULL) {
            lprintf(LOG_ERR, "auth() => strdup: Out of memory!");
            return 0;
        }
    } else {
        lprintf(LOG_ERR, "auth() => Answer does not contain \"refresh_token\" value!");
        return 0;
    }
    
    return 1;
    
}

int refresh_token() {
    memory_t mem;
    mem.memory = malloc(1);
    mem.size = 0;
    
    if(mem.memory == NULL) {
        lprintf(LOG_ERR, "refresh_token() => Out of memory!");
        return 0;
    }
    
    
    if(token_initialized(token)) {
       lprintf(LOG_ERR, "refresh_token() => Invalid token!");
       return 0;
    }
    
    size_t postfields_len = (strlen("client_id=" CLIENT_ID) + 
                             strlen("&scope=" SCOPE) + 
                             strlen("&refresh_token=") + strlen(token.refresh_token_str) +
                             strlen("&redirect_uri=") + 
                                strlen(token.redirect_uri_str) +
                             strlen("&grant_type=" GRANT_TYPE_REFRESH) + 1 
                            ) * sizeof(char);
    
    char *postfields = malloc(postfields_len);
    if(postfields == NULL) {
        lprintf(LOG_ERR, "refresh_token() => Post fields malloc error!");
        return 0;
    }
    
    postfields[0] = 0;
    
    strcat(postfields, "client_id=" CLIENT_ID);
    strcat(postfields, "&scope=" SCOPE);
    strcat(postfields, "&refresh_token=");
    strcat(postfields, token.refresh_token_str);
    strcat(postfields, "&redirect_uri=");
    strcat(postfields, token.redirect_uri_str);
    strcat(postfields, "&grant_type=" GRANT_TYPE_REFRESH);
    
    time_t created = time(NULL);
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, TOKEN_URI);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&mem);
    CURLcode res = curl_easy_perform(curl);
    free(postfields);
    
    if(res != CURLE_OK) {
        lprintf(LOG_ERR, "refresh_token() => Curl perfom error %d\n", res);
        curl_easy_cleanup(curl);
        return 0;   
    }
    curl_easy_cleanup(curl);
    
    lprintf(LOG_DEBUG, "refresh_token() => JSON string: %s", mem.memory);
    
    enum json_tokener_error jerr;
    struct json_object *answ_jobj, *err_jobj;
    
    answ_jobj = json_tokener_parse_verbose(mem.memory, &jerr);
    free(mem.memory);
    
    if(answ_jobj == NULL) {
        lprintf(LOG_ERR, "refresh_token() => JSON parse error!\n"
                         "Error description: %s\n", json_tokener_error_desc(jerr));
        return 0;
    }
    
    if(json_object_object_get_ex(answ_jobj, "error", &err_jobj)) {
        lprintf(LOG_ERR, "refresh_token() => Auth error: %s", json_object_get_string(err_jobj));
        struct json_object *err_desc_jobj;
        if(json_object_object_get_ex(answ_jobj, "error_description", &err_desc_jobj)) {
            lprintf(LOG_ERR, "Error description: %s", json_object_get_string(err_desc_jobj));
        }
        return 0;
    }
    
    struct json_object *token_type_jobj, *scope_jobj, *expires_in_jobj,
            *access_token_jobj, *refresh_token_jobj;
    
    char *token_type_str, *scope_str, *access_token_str, *refresh_token_str;
    int64_t expires_in;
    
    if(json_object_object_get_ex(answ_jobj, "token_type", &token_type_jobj)) {
        token_type_str = strdup(json_object_get_string(token_type_jobj));
        if(token_type_str == NULL) {
            lprintf(LOG_ERR, "refresh_token() => strdup: Out of memory!");
            return 0;
        }
    } else {
        lprintf(LOG_ERR, "refresh_token() => Answer does not contain \"token_type\" value!");
        return 0;
    }
    
    if(json_object_object_get_ex(answ_jobj, "scope", &scope_jobj)) {
        scope_str = strdup(json_object_get_string(scope_jobj));
        if(scope_str == NULL) {
            lprintf(LOG_ERR, "refresh_token() => strdup: Out of memory!");
            return 0;
        }
    } else {
        lprintf(LOG_ERR, "refresh_token() => Answer does not contain \"scope\" value!");
        return 0;
    }
    
    if(json_object_object_get_ex(answ_jobj, "expires_in", &expires_in_jobj)) {
        expires_in = json_object_get_int64(expires_in_jobj);
    } else {
        lprintf(LOG_ERR, "refresh_token() => Answer does not contain \"expires_in\" value!");
        return 0;
    }
    
    if(json_object_object_get_ex(answ_jobj, "access_token", &access_token_jobj)) {
        access_token_str = strdup(json_object_get_string(access_token_jobj));
        if(access_token_str == NULL) {
            lprintf(LOG_ERR, "refresh_token() => strdup: Out of memory!");
            return 0;
        }
    } else {
        lprintf(LOG_ERR, "refresh_token() => Answer does not \"access_token\" value!");
        return 0;
    }
    
    if(json_object_object_get_ex(answ_jobj, "refresh_token", &refresh_token_jobj)) {
        refresh_token_str = strdup(json_object_get_string(refresh_token_jobj));
        if(refresh_token_str == NULL) {
            lprintf(LOG_ERR, "refresh_token() => strdup: Out of memory!");
            return 0;
        }
    } else {
        lprintf(LOG_ERR, "refresh_token() => Answer does not contain \"refresh_token\" value!");
        return 0;
    }
    
    free(token.access_token_str);
    free(token.refresh_token_str);
    free(token.scope_str);
    free(token.token_type_str);
    
    token.access_token_str = access_token_str;
    token.refresh_token_str = refresh_token_str;
    token.scope_str = scope_str;
    token.token_type_str = token_type_str;
    
    token.created = created;
    token.expires_in = expires_in;
    
    return 1; 
}

char *get_access_token() {
    if(token_initialized(token)) {
        if(!load_token()) return NULL;
    }
    
    if((token.created + token.expires_in - REFRESH_BUFFER) > time(NULL)) {
        return token.access_token_str;
    }
    
    if(refresh_token(token)) {
        save_token(token);
        return token.access_token_str;
    }
    return NULL;
}

struct curl_slist *append_access_token(struct curl_slist *list) {
    char *access_token = get_access_token();
    if(access_token == NULL) {
        return NULL;
    }
    
    char *auth_header = malloc((strlen("Authorization: bearer ") + strlen(access_token) + 1) * sizeof(char));
    
    if(auth_header == NULL) {
        lprintf(LOG_ERR, "append_access_token() => Not enough memory!");
        return NULL;
    }
    
    strcpy(auth_header, "Authorization: bearer ");
    strcat(auth_header, access_token);
    
    list = curl_slist_append(list, auth_header);
    free(auth_header);
    if(list == NULL) {
        lprintf(LOG_ERR, "append_access_token() => Can't append auth_header to list!");
        return NULL;
    }
    return list;
}