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

#ifndef _G710P_H_
#define _G710P_H_

#include <stdint.h>
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define G710P_VENDOR_ID  0x046D  /**< The Logitech vendor ID. */
#define G710P_PRODUCT_ID  0xC24D  /**< The G710+ product ID. */
#define G710P_INTERFACE  1  /**< The auxiliary USB interface. */

#define G710P_REPORT_MEDIA_KEYS  0x02  /**< The media keys report type. */
#define G710P_REPORT_G_KEYS  0x03  /**< The G keys report type. */
#define G710P_REPORT_CNTRL_KEYS  0x04  /**< The control keys report type. */
#define G710P_REPORT_M_LEDS  0x06  /**< The M keys LED report type. */
#define G710P_REPORT_BL_LVLS  0x08  /**< The backlight levels report type. */

#define G710P_KEY_NEXT  (1 << 0)  /**< The next key for media. */
#define G710P_KEY_PREV  (1 << 1)  /**< The previous key for media. */
#define G710P_KEY_STOP  (1 << 2)  /**< The stop key for media. */
#define G710P_KEY_PLAY  (1 << 3)  /**< The play key for media. */
#define G710P_KEY_VLUP  (1 << 4)  /**< The volume up key for media. */
#define G710P_KEY_VLDN  (1 << 5)  /**< The volume key button for media. */
#define G710P_KEY_MUTE  (1 << 6)  /**< The mute key for media. */

#define G710P_KEY_MASK_M  (0xF << 4)  /**< The bit-mask for the M keys. */
#define G710P_KEY_M1  (1 << 4)  /**< The M1 key. */
#define G710P_KEY_M2  (1 << 5)  /**< The M2 key. */
#define G710P_KEY_M3  (1 << 6)  /**< The M3 key. */
#define G710P_KEY_MR  (1 << 7)  /**< The MR key. */

#define G710P_KEY_MASK_G  (0x3F << 8)  /**< The bit-mask for the G keys. */
#define G710P_KEY_G1  (1 << 8)  /**< The G1 key. */
#define G710P_KEY_G2  (1 << 9)  /**< The G2 key. */
#define G710P_KEY_G3  (1 << 10)  /**< The G3 key. */
#define G710P_KEY_G4  (1 << 11)  /**< The G4 key. */
#define G710P_KEY_G5  (1 << 12)  /**< The G5 key. */
#define G710P_KEY_G6  (1 << 13)  /**< The G6 key. */


/** Device handle of a supported device. */
typedef struct g710p_device g710p_device_t;

/** Report for a keyboard event. */
typedef struct g710p_report g710p_report_t;


/**
 * Report for a keyboard event.
 */
struct g710p_report
{
    uint8_t type;  /**< The report type. */
    uint8_t media_keys;  /**< The OR'd media keys. */
    uint16_t g_keys;  /**< The OR'd G keys. */
    uint8_t kb_level;  /**< The keyboard backlight level. */
    uint8_t wasd_level;  /**< The WASD backlight level. */
};


int
g710p_init(void);

int
g710p_exit(void);

const wchar_t *
g710p_error(g710p_device_t *dev);

char **
g710p_device_list_get(void);

void
g710p_device_list_free(char **devlist);

g710p_device_t *
g710p_open(const char *path);

void
g710p_close(g710p_device_t *dev);

int
g710p_report_get(g710p_device_t *dev, g710p_report_t *report, int timeout);

int
g710p_backlight_get_levels(g710p_device_t *dev, uint8_t *kb, uint8_t *wasd);

int
g710p_backlight_set_levels(g710p_device_t *dev, uint8_t kb, uint8_t wasd);

int
g710p_mkeys_get_leds(g710p_device_t *dev, uint8_t *keys);

int
g710p_mkeys_set_leds(g710p_device_t *dev, uint8_t keys);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* _G710P_H_ */
