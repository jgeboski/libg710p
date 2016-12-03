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

#include <signal.h>
#include <stdlib.h>

#include "g710p-tools-common.h"


static int quit = 0;


static void
sighandler(int signal)
{
    quit = 1;
}

int
main(int argc, const char *argv[])
{
    g710p_report_t report;
    g710p_tools_device_t *tdev;
    g710p_tools_device_t *tdevs;
    uint8_t keys;
    unsigned int n;

    tdevs = g710p_tools_devices_open();

    if (tdevs == NULL) {
        return EXIT_FAILURE;
    }

    for (n = 1, tdev = tdevs; tdev != NULL; n++, tdev = tdev->next) {
        if (!g710p_backlight_set_levels(tdev->dev, 4, 0)) {
            g710p_tools_errorln("Failed to set backlight for device %u", n);
        }
    }

    signal(SIGINT, sighandler);

    while (!quit) {
        for (n = 1, tdev = tdevs; tdev != NULL; n++, tdev = tdev->next) {
            if (!g710p_report_get(tdev->dev, &report, 100)) {
                continue;
            }

            g710p_tools_println("Device %u:", n);
            g710p_tools_println("  Report type: 0x%0x", report.type);
            g710p_tools_println("  Media Keys: 0x%0x", report.media_keys);
            g710p_tools_println("  G Keys: 0x%0x", report.g_keys);
            g710p_tools_println("  Keyboard Level: %u", report.kb_level);
            g710p_tools_println("  WASD Level: %u", report.wasd_level);
            g710p_tools_println("");

            if (!g710p_mkeys_get_leds(tdev->dev, &keys)) {
                g710p_tools_errorln("Failed to get LEDs for device %u", n);
            }

            keys ^= report.g_keys & G710P_KEY_MASK_M;

            if (!g710p_mkeys_set_leds(tdev->dev, keys)) {
                g710p_tools_errorln("Failed to set LEDs for device %u", n);
            }
        }
    }

    g710p_tools_devices_close(tdevs);
    return EXIT_SUCCESS;
}
