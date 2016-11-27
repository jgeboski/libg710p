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

/** @file */

#include <assert.h>
#include <hidapi.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "g710p.h"

/**
 * Determines if a \c hid_device_info is supported by this library.
 *
 * @param dev The hid_device_info.
 * @returns \c 1 if the device is supported, otherwise \c 0.
 */
#define G710P_DEVICE_SUPPORTED(dev) ( \
        ((dev)->vendor_id == G710P_VENDOR_ID) && \
        ((dev)->product_id == G710P_PRODUCT_ID) && \
        ((dev)->interface_number == G710P_INTERFACE) && \
        ((dev)->path != NULL) \
    )


/**
 * Internals of #g710p_device.
 */
struct g710p_device
{
    hid_device *handle;  /**< The \c hid_device. */
};


static int g710p_inited = 0;


static void
g710p_errorln(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

static char *
g710p_strdup(const char *str)
{
    char *ret;
    size_t len;

    if (str == NULL) {
        return NULL;
    }

    len = strlen(str) + 1;
    ret = malloc((sizeof *ret) * len);
    assert(ret != NULL);
    return memcpy(ret, str, len);
}

/**
 * Initializes the library. This must be called at least once before
 * using the library. It is safe to call this function more than once.
 *
 * @return \c 1 if the library successfully initialized, otherwise \c 0.
 */
int
g710p_init(void)
{
    if (g710p_inited) {
        return 1;
    }

    g710p_inited = 1;
    return hid_init() == 0;
}

/**
 * Exits the library by cleaning up everything that was initialized by
 * #g710p_init(). It is safe to call this function more than once.
 *
 * @return \c 1 if the library successfully exited, otherwise \c 0.
 */
int
g710p_exit(void)
{
    if (!g710p_inited) {
        return 1;
    }

    g710p_inited = 0;
    return hid_exit() == 0;
}

/**
 * Gets the most recent error description.
 *
 * @param dev The #g710p_device.
 * @return The error description or \c NULL if there is no error.
 */
const wchar_t *
g710p_error(g710p_device_t *dev)
{
    assert(g710p_inited);
    assert(dev != NULL);
    return hid_error(dev->handle);
}

/**
 * Gets the list of device paths. This can be used with #g710p_open()
 * to open all of the supported devices on the system. The returned
 * list should be freed with #g710p_device_list_free() when no longer
 * needed.
 *
 * @return The \c NULL terminated list of device paths.
 */
char **
g710p_device_list_get(void)
{
    size_t i;
    struct hid_device_info *dev;
    struct hid_device_info *devs;
    char **devlist;

    assert(g710p_inited);
    devs = hid_enumerate(G710P_VENDOR_ID, G710P_PRODUCT_ID);
    assert(devs != NULL);

    for (i = 1, dev = devs; dev != NULL; dev = dev->next) {
        if (G710P_DEVICE_SUPPORTED(dev)) {
            i++;
        }
    }

    devlist = malloc((sizeof *devlist) * i);
    assert(devlist != NULL);

    for (i = 0, dev = devs; dev != NULL; dev = dev->next) {
        if (G710P_DEVICE_SUPPORTED(dev)) {
            devlist[i++] = g710p_strdup(dev->path);
        }
    }

    hid_free_enumeration(devs);
    devlist[i] = NULL;
    return devlist;
}

/**
 * Frees all of the memory used by a list of device paths.
 *
 * @param devlist The list of device paths.
 */
void
g710p_device_list_free(char **devlist)
{
    size_t i;

    assert(devlist != NULL);

    for (i = 0; devlist[i] != NULL; i++) {
        free(devlist[i]);
    }

    free(devlist);
}

/**
 * Opens a supported device by its path.
 *
 * @param path The path of the device.
 * @return The #g710p_device or \c NULL on error.
 */
g710p_device_t *
g710p_open(const char *path)
{
    g710p_device_t *dev;
    hid_device *handle;

    assert(g710p_inited);
    assert(path != NULL);
    handle = hid_open_path(path);

    if (handle == NULL) {
        g710p_errorln("Failed to open %s", path);
        return NULL;
    }

    /* TODO: Check if the device is actually supported. This does not
     * seem like a trivial task given the restricted HID API. One
     * solution might be to check for the device in the device list
     * returned by g710p_device_list_get(). For now, assume the caller
     * passes the path to a supported device.
     */

    dev = malloc(sizeof *dev);
    assert(dev != NULL);
    dev->handle = handle;
    return dev;
}

/**
 * Closes a #g710p_device. The frees all resources used by the device.
 *
 * @param dev The #g710p_device.
 */
void
g710p_close(g710p_device_t *dev)
{
    assert(g710p_inited);
    assert(dev != NULL);
    hid_close(dev->handle);
    free(dev);
}

static int
g710p_read_check(int total, int required)
{
    if (required != total) {
        g710p_errorln("Expected to read %d bytes", required);
        return 0;
    }

    return 1;
}

/**
 * Populates a #g710p_report with a report read from the device. If
 * \p timeout is \c -1, this function blocks until there is something
 * to read. If \p timeout is \c 0, the function does not block.
 *
 * @param dev The #g710p_device.
 * @param report The #g710p_report.
 * @param timeout The timeout in milliseconds.
 * @return \c 1 if the report was successfully read, otherwise \c 0.
 */
int
g710p_report_get(g710p_device_t *dev, g710p_report_t *report, int timeout)
{
    int res;
    uint8_t data[8];

    assert(g710p_inited);
    assert(dev != NULL);
    assert(report != NULL);

    res = hid_read_timeout(dev->handle, data, sizeof data, timeout);

    if (res == -1) {
        g710p_errorln("Failed to read data");
        return 0;
    }

    if (res < 2) {
        return 0;
    }

    memset(report, 0, sizeof *report);
    report->type = data[0];

    switch (report->type) {
    case G710P_REPORT_MEDIA_KEYS:
        if (g710p_read_check(res, 2)) {
            report->media_keys = data[1];
            return 1;
        }
        break;

    case G710P_REPORT_G_KEYS:
        if (g710p_read_check(res, 4)) {
            report->g_keys = (data[1] << 8) | data[2];
            return 1;
        }
        break;

    case G710P_REPORT_CNTRL_KEYS:
        if (g710p_read_check(res, 8)) {
            report->kb_level = data[3];
            report->wasd_level = data[2];
            return 1;
        }
        break;
    }

    return 0;
}

/**
 * Gets the backlight brightness levels of the keyboard. Where \c 0 is
 * the brightest and \c 4 is the darkest.
 *
 * @param dev The #g710p_device.
 * @param kb The return location for the keyboard level.
 * @param wasd The return location for the WASD level.
 * @return \c 1 if the levels were successfully returned, otherwise \c 0.
 */
int
g710p_backlight_get_levels(g710p_device_t *dev, uint8_t *kb, uint8_t *wasd)
{
    int res;

    uint8_t data[4] = {
        G710P_REPORT_BL_LVLS,
        0x00,
        0x00,
        0x00
    };

    assert(g710p_inited);
    assert(dev != NULL);
    assert(kb != NULL);
    assert(wasd != NULL);

    res = hid_get_feature_report(dev->handle, data, sizeof data);

    if (res != sizeof data) {
        return 0;
    }

    *kb = data[2];
    *wasd = data[1];
    return 1;
}

/**
 * Sets the backlight brightness levels of the keyboard. Where \c 0 is
 * the brightest and \c 4 is the darkest.
 *
 * @param dev The #g710p_device.
 * @param kb The keyboard level.
 * @param wasd The WASD level.
 * @return 1 if the levels were successfully set, otherwise 0.
 */
int
g710p_backlight_set_levels(g710p_device_t *dev, uint8_t kb, uint8_t wasd)
{
    int res;

    uint8_t data[4] = {
        G710P_REPORT_BL_LVLS,
        wasd,
        kb,
        0x00
    };

    assert(g710p_inited);
    assert(dev != NULL);
    assert(kb <= 4);
    assert(wasd <= 4);

    res = hid_send_feature_report(dev->handle, data, sizeof data);
    return res == sizeof data;
}

/**
 * Gets the LED states of the M keys. The keys with active LEDs are
 * returned via \p keys.
 *
 * @param dev The #g710p_device.
 * @param keys The return location for the active M keys.
 * @return \c 1 if the states were successfully returned, otherwise \c 0.
 */
int
g710p_mkeys_get_leds(g710p_device_t *dev, uint8_t *keys)
{
    int res;

    uint8_t data[2] = {
        G710P_REPORT_M_LEDS,
        0x00
    };

    assert(g710p_inited);
    assert(dev != NULL);
    assert(keys != NULL);

    res = hid_get_feature_report(dev->handle, data, sizeof data);

    if (res != sizeof data) {
        return 0;
    }

    *keys = data[1];
    return 1;
}

/**
 * Sets the LED states of the M keys.
 *
 * @param dev The #g710p_device.
 * @param keys The active M keys.
 * @return \c 1 if the levels were successfully set, otherwise \c 0.
 */
int
g710p_mkeys_set_leds(g710p_device_t *dev, uint8_t keys)
{
    int res;

    uint8_t data[2] = {
        G710P_REPORT_M_LEDS,
        keys
    };

    assert(g710p_inited);
    assert(dev != NULL);

    res = hid_send_feature_report(dev->handle, data, sizeof data);
    return res == sizeof data;
}
