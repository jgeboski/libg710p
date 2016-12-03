/*
 * Copyright 2016 James Geboski <jgeboski@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#ifndef _G710P_TOOLS_COMMON_H_
#define _G710P_TOOLS_COMMON_H_

#include <g710p.h>


typedef struct g710p_tools_device g710p_tools_device_t;


struct g710p_tools_device
{
    g710p_tools_device_t *next;
    g710p_device_t *dev;
    uint8_t kb_level;
    uint8_t wasd_level;
    uint8_t m_keys;
};


void
g710p_tools_errorln(const char *format, ...);

void
g710p_tools_println(const char *format, ...);

g710p_tools_device_t *
g710p_tools_devices_open(void);

void
g710p_tools_devices_close(g710p_tools_device_t *tdevs);

#endif /* _G710P_TOOLS_COMMON_H_ */
