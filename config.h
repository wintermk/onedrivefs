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
 * File:   config.h
 * Author: Karl Wintermann
 *
 * Created on 17. September 2017, 13:37
 */

#ifndef CONFIG_H
#define CONFIG_H


#define CLIENT_ID               "bfdf0d2d-24ec-44f3-9063-ef993e0e7153"
#define REDIRECT_URI_APP        "msalbfdf0d2d-24ec-44f3-9063-ef993e0e7153%3A%2F%2Fodfs%2Fauth"
#define REDIRECT_URI_BROWSER    "https%3A%2F%2Flogin.microsoftonline.com%2Fcommon%2Foauth2%2Fnativeclient"

#define AUTH_URI                "https://login.microsoftonline.com/common/oauth2/v2.0/authorize"
#define TOKEN_URI               "https://login.microsoftonline.com/common/oauth2/v2.0/token"

#define RESPONSE_TYPE           "code"
#define RESPONSE_MODE           "query"

#define SCOPE                   "files.readwrite.all%20offline_access"

#define GRANT_TYPE_AUTH         "authorization_code"
#define GRANT_TYPE_REFRESH      "refresh_token"

#define REFRESH_BUFFER          60





#endif /* CONFIG_H */

