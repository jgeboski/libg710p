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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "g710p-tools-common.h"


void
g710p_tools_errorln(const char *format, ...)
{
    va_list ap;

    fprintf(stderr, "error: ");
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

void
g710p_tools_println(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
    printf("\n");
}

g710p_tools_device_t *
g710p_tools_devices_open(void)
{
    char **devlist;
    const char *path;
    g710p_device_t *dev;
    g710p_tools_device_t *tail = NULL;
    g710p_tools_device_t *tdev;
    g710p_tools_device_t *tdevs = NULL;
    unsigned int i;
    unsigned int n = 1;

    if (!g710p_init()) {
        g710p_tools_errorln("Failed to initialize libg710p");
        return NULL;
    }

    devlist = g710p_device_list_get();

    for (i = 0; devlist[i] != NULL; i++) {
        path = devlist[i];
        g710p_tools_println("Opening %s as device %u", path, n);
        dev = g710p_open(path);

        if (dev == NULL) {
            g710p_tools_errorln("Failed to open %s, skipping", path);
            continue;
        }

        tdev = calloc(1, sizeof *tdev);
        assert(tdev != NULL);
        tdev->dev = dev;
        n++;

        if (tail != NULL) {
            tail->next = tdev;
            tail = tdev;
        } else {
            tdevs = tdev;
            tail = tdev;
        }

        if (!g710p_backlight_get_levels(dev, &tdev->kb_level, &tdev->wasd_level)) {
            g710p_tools_errorln("Failed to get bl levels for device %u", n);
        }

        if (!g710p_mkeys_get_leds(dev, &tdev->m_keys)) {
            g710p_tools_errorln("Failed to get LED states for device %u", n);
        }
    }

    if (tdevs == NULL) {
        g710p_tools_errorln("Failed to find any supported devices");
    }

    g710p_device_list_free(devlist);
    return tdevs;
}

void
g710p_tools_devices_close(g710p_tools_device_t *tdevs)
{
    g710p_tools_device_t *tdev;
    unsigned int n;

    for (n = 1; tdevs != NULL; n++) {
        tdev = tdevs;
        tdevs = tdevs->next;

        if (!g710p_backlight_set_levels(tdev->dev, tdev->kb_level, tdev->wasd_level)) {
            g710p_tools_errorln("Failed to set bl levels for device %u", n);
        }

        if (!g710p_mkeys_set_leds(tdev->dev, tdev->m_keys)) {
            g710p_tools_errorln("Failed to set LED states for device %u", n);
        }

        g710p_close(tdev->dev);
        free(tdev);
    }

    if (!g710p_exit()) {
        g710p_tools_errorln("Failed to exit libg710p");
    }
}
