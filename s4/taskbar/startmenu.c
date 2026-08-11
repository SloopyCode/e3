/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: startmenu.c
 *
 */
#include "startmenu.h"
#include "../compositor/comp.h"
#include "../cfg.h"
#include "../fonts/fonts.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

static int menu_open = 0;
static int menu_x = 0;
static int menu_y = 0;
static int screen_width = 0;
static int screen_height = 0;

static int _slen(const char *string)
{
    int length = 0;
    while (string[length]) length++;
    return length;
}

static int file_exists(const char *filepath)
{
    int file_descriptor = open(filepath, O_RDONLY);
    if (file_descriptor < 0) return 0;
    close(file_descriptor);
    return 1;
}

static void launch_poweroff(void)
{
	printf(":: sm: launching poweroff...");
    if (!file_exists(POWEROFF_LAUNCHPAD_PATH)) return;
    spawn(POWEROFF_LAUNCHPAD_PATH);
}

static void launch_reboot(void)
{
	printf(":: sm: launching reboot...");
    if (!file_exists(REBOOT_LAUNCHPAD_PATH)) return;
    spawn(REBOOT_LAUNCHPAD_PATH);
}

static void draw_label(
    const char *label,
    int x,
    int y,
    int width,
    int height,
    unsigned int foreground,
    unsigned int background
) {
    int font_width = font_w(FONT8X12_BOLD);
    int font_height = font_h(FONT8X12_BOLD);
    int label_length = _slen(label);
    int text_width = label_length * font_width;
    int text_x = x + (width - text_width) / 2;
    int text_y = y + (height - font_height) / 2;

    for (int char_index = 0; char_index < label_length; char_index++)
    {
        unsigned char character = (unsigned char)label[char_index] & 0x7Fu;
        for (int row_index = 0; row_index < font_height; row_index++)
        {
            uint16_t glyph_bits = font_glyph(FONT8X12_BOLD, character, row_index);
            for (int col_index = 0; col_index < font_width; col_index++)
            {
                unsigned int pixel_color = (glyph_bits & (1u << col_index)) ? foreground : background;
                comp_set(text_x + char_index * font_width + col_index, text_y + row_index, pixel_color);
            }
        }
    }
}

static void draw_sm_button(
    const char *label,
    int x,
    int y,
    int width,
    int height,
    int hover,
    int pressed
) {
    unsigned int button_face = TB_BUTTON_BG;

    comp_fill(x, y, width, height, button_face);

    if (hover || pressed)
    {
        unsigned int border_color = pressed ? TB_BTN_TOP : TB_LIGHT;

        comp_fill(x, y, width, TB_BORDER_W, border_color);
        comp_fill(x, y + height - TB_BORDER_W, width, TB_BORDER_W, border_color);
        comp_fill(x, y, TB_BORDER_W, height, border_color);
        comp_fill(x + width - TB_BORDER_W, y, TB_BORDER_W, height, border_color);

        comp_fill(x, y, width, height, button_face);
        comp_fill(x, y, width, 2, TB_TOP_BORDER);
    }

    draw_label(label, x, y, width, height, TB_WHITE, button_face);
}

static int hit_popup(int mouse_x, int mouse_y)
{
    return
    	mouse_x >= menu_x && mouse_x < menu_x + SM_W &&
        mouse_y >= menu_y && mouse_y < menu_y + SM_H
    ;
}

static void reboot_rect(int *out_x, int *out_y, int *out_width, int *out_height)
{
    *out_x = menu_x + SM_BTN_PAD;
    *out_y = menu_y + SM_BTN_ROW_Y;
    *out_width = SM_BTN_W;
    *out_height = SM_BTN_H;
}

static void poweroff_rect(int *out_x, int *out_y, int *out_width, int *out_height)
{
    *out_x = menu_x + SM_BTN_PAD * 2 + SM_BTN_W;
    *out_y = menu_y + SM_BTN_ROW_Y;
    *out_width = SM_BTN_W;
    *out_height = SM_BTN_H;
}

static int hit_rect(int mouse_x, int mouse_y, int x, int y, int width, int height)
{
    return mouse_x >= x && mouse_x < x + width && mouse_y >= y && mouse_y < y + height;
}

void startmenu_init(int width, int height)
{
    screen_width = width;
    screen_height = height;
    menu_open = 0;
}

void startmenu_toggle(int button_x, int taskbar_y)
{
    if (menu_open)
    {
        menu_open = 0;
        return;
    }

    menu_x = button_x;
    menu_y = taskbar_y - SM_H - SM_MARGIN;

    if (menu_x + SM_W > screen_width) menu_x = screen_width - SM_W;
    if (menu_x < 0) menu_x = 0;
    if (menu_y < 0) menu_y = 0;

    menu_open = 1;
}

void startmenu_close(void)
{
    menu_open = 0;
}

int startmenu_is_open(void)
{
    return menu_open;
}

void startmenu_draw(int mouse_x, int mouse_y, int button_down)
{
    if (!menu_open) return;

    comp_fill(menu_x, menu_y, SM_W, SM_H, TB_BACKGROUND);

    comp_fill(menu_x, menu_y, SM_W, 1, TB_TOP_BORDER);
    comp_fill(menu_x, menu_y + SM_H - 1, SM_W, 1, TB_TOP_BORDER);
    comp_fill(menu_x, menu_y, 1, SM_H, TB_TOP_BORDER);
    comp_fill(menu_x + SM_W - 1, menu_y, 1, SM_H, TB_TOP_BORDER);

    comp_fill(
        menu_x + 1,
        menu_y + SM_BTN_ROW_Y - TB_BTN_PAD,
        SM_W - 2, 1,
        TB_TOP_BORDER
    );

    int reboot_x, reboot_y, reboot_width, reboot_height;
    int poweroff_x, poweroff_y, poweroff_width, poweroff_height;
    reboot_rect(&reboot_x, &reboot_y, &reboot_width, &reboot_height);
    poweroff_rect(&poweroff_x, &poweroff_y, &poweroff_width, &poweroff_height);

    int reboot_hover = hit_rect(mouse_x, mouse_y, reboot_x, reboot_y, reboot_width, reboot_height);
    int reboot_pressed = reboot_hover && button_down;
    int poweroff_hover = hit_rect(mouse_x, mouse_y, poweroff_x, poweroff_y, poweroff_width, poweroff_height);
    int poweroff_pressed = poweroff_hover && button_down;

    draw_sm_button("reboot", reboot_x, reboot_y, reboot_width, reboot_height, reboot_hover, reboot_pressed);
    draw_sm_button("power off", poweroff_x, poweroff_y, poweroff_width, poweroff_height, poweroff_hover, poweroff_pressed);
}

int startmenu_click(int mouse_x, int mouse_y)
{
    if (!menu_open) return 0;

    if (!hit_popup(mouse_x, mouse_y)) return 0;

    {
        int x, y, width, height;
        reboot_rect(&x, &y, &width, &height);
        if (hit_rect(mouse_x, mouse_y, x, y, width, height))
        {
            menu_open = 0;
            launch_reboot();
            return 1;
        }
    }

    {
        int x, y, width, height;
        poweroff_rect(&x, &y, &width, &height);
        if (hit_rect(mouse_x, mouse_y, x, y, width, height))
        {
            menu_open = 0;
            launch_poweroff();
            return 1;
        }
    }

    return 1;
}
