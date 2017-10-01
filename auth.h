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
 * File:   auth.h
 * Author: Karl Wintermann
 *
 * Created on 17. September 2017, 12:23
 */

#ifndef AUTH_H
#define AUTH_H

#include <curl/curl.h>

void auth_init(char *token_dir);
void auth_cleanup();

int auth(char *redirect_uri_str);

char *get_access_token();

struct curl_slist *append_access_token(struct curl_slist *list);

#endif /* AUTH_H */
