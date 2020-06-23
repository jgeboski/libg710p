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

#include <argp.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pulse/pulseaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "g710p-tools-common.h"


#define LEVEL_CNT  5
#define LEVEL_MAX  4
#define PEAK_MIN  128


typedef struct user_data user_data_t;


struct user_data
{
    g710p_tools_device_t *tdevs;
    int daemonize;
    int verbose;
    pa_mainloop_api *mlapi;
    pa_stream *s;
    uint8_t peak_max;
    uint32_t idx;
};


const char *argp_program_version = PACKAGE_STRING;
const char *argp_program_bug_address = PACKAGE_BUGREPORT;


static void
keyboard_set_leds(g710p_tools_device_t *tdevs, uint8_t level)
{
    g710p_tools_device_t *tdev;
    uint8_t m_keys = 0;

    assert(level <= 4);

    if (level <= 3) {
        m_keys |= G710P_KEY_M1;
    }

    if (level <= 2) {
        m_keys |= G710P_KEY_M2;
    }

    if (level <= 1) {
        m_keys |= G710P_KEY_M3;
    }

    if (level == 0) {
        m_keys |= G710P_KEY_MR;
    }

    for (tdev = tdevs; tdev != NULL; tdev = tdev->next) {
        g710p_mkeys_set_leds(tdev->dev, m_keys);
        g710p_backlight_set_levels(tdev->dev, level, level);
    }
}

static void
stream_read_callback(pa_stream *s, size_t len, void *userdata)
{
    const uint8_t *data;
    uint8_t level = LEVEL_MAX;
    uint8_t sample;
    uint8_t span;
    user_data_t *udata = userdata;

    if (pa_stream_peek(s, (void *) &data, &len) < 0) {
        g710p_tools_errorln("Failed to read stream");
        udata->mlapi->quit(udata->mlapi, EXIT_FAILURE);
        return;
    }

    if (data == NULL) {
        if (len != 0) {
            pa_stream_drop(s);
        }

        return;
    }

    span = udata->peak_max - PEAK_MIN;
    sample = data[len >> 1] - PEAK_MIN;

    if (sample != 0) {
        level = sample / (span / LEVEL_CNT);
    }

    if (level > LEVEL_MAX) {
        level = LEVEL_MAX;
    }

    if (udata->verbose) {
        g710p_tools_println(
            "Sample: %u (Max: %u), Level: %u",
            sample,
            span,
            level
        );
    }

    keyboard_set_leds(udata->tdevs, level);
    pa_stream_drop(s);
}

static void
context_state_callback(pa_context *ctx, void *userdata)
{
    char dev[11];
    int res;
    pa_context_state_t state;
    pa_stream *s;
    user_data_t *udata = userdata;

    static const pa_buffer_attr attr = {
        .maxlength = (uint32_t) -1,
        .fragsize = sizeof (uint8_t)
    };

    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_U8,
        .rate = 50,
        .channels = 1
    };

    static const pa_stream_flags_t flags =
        PA_STREAM_ADJUST_LATENCY |
        PA_STREAM_DONT_INHIBIT_AUTO_SUSPEND |
        PA_STREAM_DONT_MOVE |
        PA_STREAM_PEAK_DETECT;

    state = pa_context_get_state(ctx);

    if (state != PA_CONTEXT_READY) {
        return;
    }

    s = pa_stream_new(ctx, "Peak detector", &ss, NULL);

    if (s == NULL) {
        g710p_tools_errorln("Failed to create detector stream");
        udata->mlapi->quit(udata->mlapi, EXIT_FAILURE);
        return;
    }

    udata->s = s;
    sprintf(dev, "%u", udata->idx);
    pa_stream_set_read_callback(s, stream_read_callback, udata);
    res = pa_stream_connect_record(s, dev, &attr, flags);

    if (res < 0) {
        g710p_tools_errorln("Failed to connect detector stream");
        udata->mlapi->quit(udata->mlapi, EXIT_FAILURE);
    }
}

static void
signal_callback(
    pa_mainloop_api *mlapi,
    pa_signal_event *event,
    int signal,
    void *userdata)
{
    mlapi->quit(mlapi, EXIT_SUCCESS);
}

static int
daemonize(void)
{
    int fd;

    switch (fork()) {
    case 0:
        break;

    case -1:
        return 0;

    default:
        exit(EXIT_SUCCESS);
    }

    fd = open("/dev/null", O_RDWR);

    if (fd == -1) {
        return 0;
    }

    return (setsid() != -1) &&
           (chdir("/") != -1) &&
           (dup2(fd, STDIN_FILENO) != -1) &&
           (dup2(fd, STDOUT_FILENO) != -1) &&
           (dup2(fd, STDERR_FILENO) != -1);
}

static error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
    user_data_t *udata = state->input;

    switch (key) {
    case 'd':
        udata->daemonize = 1;
        break;

    case 'p':
        udata->peak_max = atoi(arg);

        if (udata->peak_max < (PEAK_MIN + LEVEL_CNT)) {
            udata->peak_max = PEAK_MIN + LEVEL_CNT;
        }
        break;

    case 'v':
        udata->verbose = 1;
        break;

    case ARGP_KEY_INIT:
        udata->peak_max = PEAK_MIN + 64;
        break;

    case ARGP_KEY_ARG:
        if (state->arg_num != 0) {
            argp_usage(state);
        }

        udata->idx = atoi(arg);
        break;

    case ARGP_KEY_END:
        if (state->arg_num != 1) {
            argp_usage(state);
        }

        if (udata->daemonize && udata->verbose) {
            udata->verbose = 0;
        }
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

int
main(int argc, char *argv[])
{
    g710p_tools_device_t *tdevs;
    int res;
    int ret = EXIT_SUCCESS;
    pa_context *ctx;
    pa_mainloop *ml;
    pa_mainloop_api *mlapi;
    user_data_t udata;

    static const struct argp_option options[] = {
        {"daemonize", 'd', NULL, 0, "Fork the process to the background", 0},
        {"peak-max", 'p', "MAX", 0, "Maximum PCM value (133 <= x <= 255)", 0},
        {"verbose", 'v', NULL, 0, "Verbosely print additional messages", 0},
        {NULL}
    };

    static const struct argp argp = {
        options,
        parse_opt,
        "<idx>",
        "Sets the G710+ LEDs according to the PulseAudio PCM data",
        NULL,
        NULL,
        NULL
    };

    memset(&udata, 0, sizeof udata);
    argp_parse(&argp, argc, argv, 0, NULL, &udata);
    tdevs = g710p_tools_devices_open();

    if (tdevs == NULL) {
        return EXIT_FAILURE;
    }

    /* Daemonize before initializing PulseAudio */
    if (udata.daemonize && !daemonize()) {
        g710p_tools_errorln("Failed to daemonize");
        return EXIT_FAILURE;
    }

    udata.tdevs = tdevs;
    ml = pa_mainloop_new();
    assert(ml != NULL);

    mlapi = pa_mainloop_get_api(ml);
    assert(mlapi != NULL);
    udata.mlapi = mlapi;

    ctx = pa_context_new(mlapi, __FILE__);
    assert(ctx != NULL);
    res = pa_context_connect(ctx, NULL, PA_CONTEXT_NOFLAGS, NULL);

    if (res < 0) {
        g710p_tools_errorln("Failed to connect to PulseAudio");
        ret = EXIT_FAILURE;
        goto cleanup;
    }

    res = pa_signal_init(mlapi);
    assert(res == 0);

    pa_signal_new(SIGINT, signal_callback, &udata);
    pa_signal_new(SIGTERM, signal_callback, &udata);

    pa_context_set_state_callback(ctx, context_state_callback, &udata);
    pa_mainloop_run(ml, &ret);
    pa_context_disconnect(ctx);

cleanup:
    if (udata.s != NULL) {
        pa_stream_disconnect(udata.s);
        pa_stream_unref(udata.s);
    }

    pa_context_unref(ctx);
    pa_mainloop_free(ml);
    g710p_tools_devices_close(tdevs);
    return ret;
}
