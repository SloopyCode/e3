/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: e3
 * FILE: entries.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "entries.h"
#include "../bg/bmp/bmp.h"
#include "../config/cfg.h"
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

//all app entries till we have a parser so it reads from a config
static tb_widget_t s_entries[] =
{
    {
        .type          = TB_WIDGET_APP,
        .name          = "login",
        .text          = "login",
        .exec          = "/system/desktop/login.elf",
        .icon_path     = "/system/desktop/background/2.bmp",
        .icon          = { .loaded = 0 },
        .popup_pid     = -1,
        .popup_w       = 0,
        .popup_h       = 0,
        .dirty         = 0,
    },
    {
        .type          = TB_WIDGET_APP,
        .name          = "DOOM",
        .text          = "DOOM",
        .exec          = "/bin/doomgeneric.elf",
        .icon_path     = "/system/desktop/icons/doom.bmp",
        .icon          = { .loaded = 0 },
        .popup_pid     = -1,
        .popup_w       = 0,
        .popup_h       = 0,
        .dirty         = 0,
    },
    {
        .type          = TB_WIDGET_APP,
        .name          = "gears",
        .text          = "gears",
        .exec          = "/bin/gears.elf",
        .icon_path     = NULL,
        .icon          = { .loaded = 0 },
        .popup_pid     = -1,
        .popup_w       = 0,
        .popup_h       = 0,
        .dirty         = 0,
    },
    {
        .type          = TB_WIDGET_APP,
        .name          = "welcome",
        .text          = "welcome",
        .exec          = "system/welcome.elf",
        .icon_path     = NULL,
        .icon          = { .loaded = 0 },
        .popup_pid     = -1,
        .popup_w       = 0,
        .popup_h       = 0,
        .dirty         = 0,
    },
};

#define ENTRIES_COUNT (int)(sizeof(s_entries) / sizeof(s_entries[0]))

static int does_my_file_exists(const char *path) // does it??
{
//
}

static void entry_load_icon(tb_widget_t *entry)
{
    const char *path;

    if (entry->icon_path && entry->icon_path[0] != '\0')
    {
        path = entry->icon_path;
    }
    else
    {
        // automatic by app name
        static char auto_path[256];

        snprintf(
            auto_path,
            sizeof(auto_path),
            "/system/desktop/icons/%s.bmp",
            entry->name
        );

        path = auto_path;
    }

    if (bmp_load(path, &entry->icon.image) == 0)
    {
        entry->icon.loaded = 1;
        return;
    }

    if (bmp_load("/system/desktop/icons/exec.bmp", &entry->icon.image ) == 0
    ){
    	entry->icon.loaded = 1;
    }
}

tb_widget_t *entries_get(int *out_count)
{
    if (out_count) *out_count = ENTRIES_COUNT;


    static int initialized = 0;

    if (!initialized)
    {
        for (int i = 0; i < ENTRIES_COUNT; i++) entry_load_icon(&s_entries[i]);

        initialized = 1;
    }


    return s_entries;
}

// to load icons is implemented but not used xd
// used when the user wants to change a icon per app/when running
void taskbar_load_entry_icon(int index)
{
    if (index < 0 || index >= ENTRIES_COUNT) return;
    //i dont want to have icons yet cuz bmp doesnt support transparency but in some time i add png ig
    entry_load_icon(&s_entries[index]);
}
