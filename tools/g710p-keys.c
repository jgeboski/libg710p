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
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <g710p.h>


static int quit = 0;


static void
println(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
    printf("\n");
}

static void
sighandler(int signal)
{
    quit = 1;
}

int
main(int argc, const char *argv[])
{
    char **devlist;
    g710p_device_t *dev;
    g710p_device_t **devs;
    g710p_report_t report;
    uint8_t keys;
    uint8_t kb;
    uint8_t wasd;
    unsigned int devc;
    unsigned int i;

    assert(g710p_init());
    devlist = g710p_device_list_get();
    println("Found the following HID devices:");

    for (i = 0; devlist[i] != NULL; i++) {
        println("%s", devlist[i]);
    }

    devc = i;
    devs = malloc((sizeof *devs) * devc);

    for (i = 0; i < devc; i++) {
        devs[i] = g710p_open(devlist[i]);
        assert(devs[i] != NULL);
    }

    g710p_device_list_free(devlist);
    signal(SIGINT, sighandler);

    assert(g710p_backlight_get_levels(devs[0], &kb, &wasd));
    assert(g710p_backlight_set_levels(devs[0], 4, 0));

    while (!quit) {
        for (i = 0; i < devc; i++) {
            dev = devs[i];

            if (!g710p_report_get(dev, &report, 100)) {
                continue;
            }

            println("Device %u:", i + 1);
            println("  Report type: 0x%0x", report.type);
            println("  Media Keys: 0x%0x", report.media_keys);
            println("  G Keys: 0x%0x", report.g_keys);
            println("  Keyboard Level: %u", report.kb_level);
            println("  WASD Level: %u", report.wasd_level);
            println("");

            assert(g710p_mkeys_get_leds(dev, &keys));
            keys ^= report.g_keys & G710P_KEY_MASK_M;
            assert(g710p_mkeys_set_leds(dev, keys));
        }
    }

    assert(g710p_backlight_set_levels(devs[0], kb, wasd));

    for (i = 0; i < devc; i++) {
        g710p_close(devs[i]);
    }

    assert(g710p_exit());
    return EXIT_SUCCESS;
}
