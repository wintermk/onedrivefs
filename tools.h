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
 * File:   tools.h
 * Author: Karl Wintermann
 *
 * Created on 30. September 2017, 11:08
 */

#ifndef TOOLS_H
#define TOOLS_H

#include <stddef.h>

typedef struct memory_s {
    char *memory;
    size_t size;
} memory_t;

size_t write_memory(void *contents, size_t size, size_t nmemb, void *userp);

#endif /* TOOLS_H */
