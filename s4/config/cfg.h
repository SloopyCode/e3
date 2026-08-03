/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: cfg.h
 *
 */

#pragma once

#define DT_DIR    "/tmp/dt"
#define DT_CMD    "/tmp/dt/cmd"
#define DT_DIRTY  "/tmp/dt/dirty"
#define DT_CURSOR "/tmp/dt/cursor"

#ifdef __sulfur__
	#define MOUSE_DEV    "/dev/mouse"
	#define KEYBOARD_DEV "/dev/kbd0"
	#define FRAMEBUFFER_DEV "/dev/fb0"

	#define SYSTEM "/system/desktop/" //the path to the desktop in the system
	#define CFGFILE   SYSTEM "desktop.conf"
	#define ENTRYFILE SYSTEM "entries.json"
#elif __ascentos__
    #define MOUSE_DEV    "/dev/mouse"
	#define KEYBOARD_DEV "/dev/kbd"
	#define FRAMEBUFFER_DEV "/dev/fb0"

	#define SYSTEM "/system/desktop/"
	#define CFGFILE   SYSTEM "desktop.conf"
	#define ENTRYFILE SYSTEM "entries.json"
#elif __has_include
	// idfk if this works... rn i cant test it cuz doccrOS isnt finished
	// but i think it works in newer gcc vrsions __has_include is supported ig.... :/
    #if __has_include(<emx/sinfo.h>)
        #include <emx/sinfo.h>
        #if defined(__EMEX__)
	       	#define MOUSE_DEV    "/dev/input/mouse0"
			#define KEYBOARD_DEV "/dev/input/keyboard0"
			#define FRAMEBUFFER_DEV "/dev/fb0"

			#define SYSTEM "/emr/system/desktop/"
			#define CFGFILE   SYSTEM "desktop.conf"
			#define ENTRYFILE SYSTEM "entries.json"
        #endif
    #endif
#else
    #error "Unsupported operating system"
#endif

#define DEFAULT_ICON SYSTEM "icons/exec.bmp"
#define STARTBUTTON_ICON SYSTEM "icons/start.bmp"

#define DT_BG 0xFF008080u  // win95/98 teal ig its a fallback

#define DARK_MODE 1

// rubber band selection colors
#define BAND_BORDER 0xCCA8A5A5u   // border
#define BAND_FILL   0x33A8A5A5u   // the content

//taskbar
#define TB_H 38
#define TB_BTN_PAD 3
#define TB_BTN_VPAD 6
#define TB_BTN_W 120
#define TB_BORDER_W 3
#define TB_ENTRY_NAMELEN 32
#define TB_ENTRY_EXECLEN 64
#define TB_MAX_ENTRIES 10
#define TB_START_W 22
#define TB_STARTBUTTON_PAD_LEFT 5
#define TB_STARTBUTTON_PAD_RIGHT 5

#define MAX_APPICON_SIZE 22

#define POWEROFF_LAUNCHPAD_PATH "/bin/poweroff.elf"
#define REBOOT_LAUNCHPAD_PATH "/bin/reboot.elf"

#if DARK_MODE == 1
	#define TB_BACKGROUND  0xFF202020u
	#define TB_TOP_BORDER  0xFF707070u
	#define TB_BUTTON_BG   0xFF404040u
	#define TB_BLACK       0xFF000000u
	#define TB_WHITE       0xFFFFFFFFu
	#define TB_LIGHT       0xFF808080u
	#define TB_SHADOW      0xFF606060u
	#define TB_BTN_TOP     0xFF6A89A7u
	#define TB_FACE        0xFFD4D0C8u
	#define TB_TRANSPARENT 0x00FFFFFFu

	#define WIN_BLACK           0xFF000000u
	#define WIN_WHITE           0xFF404040u
	#define WIN_FACE            0xFFD4D0C8u
	#define WIN_BUTTON          0xFFD4D0C8u
	#define WIN_FOCUSED_BG      0xFF5F5F5Fu
	#define WIN_UNFOCUSED_BG    0xFF5F5F5Fu
#elif DARK_MODE == 0
	#define TB_BACKGROUND  0xFFDFDFDFu
	#define TB_TOP_BORDER  0xFF8F8F8Fu
	#define TB_BUTTON_BG   0xFFBFBFBFu
	#define TB_BLACK       0xFFFFFFFFu
	#define TB_WHITE       0xFF000000u
	#define TB_LIGHT       0xFF7F7F7Fu
	#define TB_SHADOW      0xFF9F9F9Fu
	#define TB_BTN_TOP     0xFF957658u
	#define TB_FACE        0xFF2B2F37u
	#define TB_TRANSPARENT 0x00000000u

	#define WIN_BLACK           0xFF000000u
	#define WIN_WHITE           0xFFFFFFFFu
	#define WIN_FACE            0xFFD4D0C8u
	#define WIN_BUTTON          0xFFD4D0C8u
	#define WIN_FOCUSED_BG      0xFFA0A0A0u
	#define WIN_UNFOCUSED_BG    0xFF000000u
#endif

#define SM_W          180
#define SM_H          80
#define SM_MARGIN     2

#define SM_BTN_H      (TB_H - TB_BTN_VPAD * 2)
#define SM_BTN_W      ((SM_W - TB_BTN_PAD * 3) / 2)
#define SM_BTN_PAD    TB_BTN_PAD

#define SM_BTN_ROW_Y  (SM_H - SM_BTN_H - SM_BTN_PAD)

#define DT_ENABLE_RESIZING 1
#define ENABLE_TILING 0

// window title bar
#define DT_TITLE_ACT 0xFFB0D0C0u  // green; when focused
#define DT_TITLE_INA 0xFF808080u  // grey ; when unfocused
//#define DT_TITLE_INA 0xFF4F2F3Fu //red
#define DT_TITLE_TXT 0xFFFFFFFFu  // text of titlebar

// titlebar config
#define DT_TITLE_H  21 // tot. inc. high
#define DT_TITLE_PB 5  // pad. top
#define DT_BORDER   1  // border width

#define DT_CLOSE_X  5  // x offset
#define DT_CLOSE_Y  5  // y offset
#define DT_CLOSE_SZ 13 // square size

#define DT_MAX_X (DT_CLOSE_X + DT_CLOSE_SZ + 4)
#define DT_MAX_Y DT_CLOSE_Y
#define DT_MAX_SZ DT_CLOSE_SZ

// not minimize this button will put the window into taskbar, like fully fully minimize
#define DT_MIN_X (DT_MAX_X + DT_MAX_SZ + 4)
#define DT_MIN_Y DT_CLOSE_Y
#define DT_MIN_SZ DT_CLOSE_SZ

// font rendering
#define DT_FW 8
#define DT_FH 12

// scaling settings
#define RENDERER_SCALING_ENABLED 0
#define RENDERER_SUPERSAMPLING_FACTOR 0
