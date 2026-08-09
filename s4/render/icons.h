/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: icons.h
 *
 */

#pragma once

#define DT_ICON_W 13
#define DT_ICON_H 13

#define DT_ICON_FILL 0xFFD4D0C8u
#define DT_ICON_FILL_HOVER 0xFFFFFFFFu

#define T 0x00000000u
#define B 0xFF101010u
#define W DT_ICON_FILL

static const unsigned int dt_icon_close_px[DT_ICON_W * DT_ICON_H] = {
	B,B,B,B,B,B,B,B,B,B,B,B,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,B,W,W,W,W,W,B,W,W,B,
	B,W,W,W,B,W,W,W,B,W,W,W,B,
	B,W,W,W,W,B,W,B,W,W,W,W,B,
	B,W,W,W,W,W,B,W,W,W,W,W,B,
	B,W,W,W,W,B,W,B,W,W,W,W,B,
	B,W,W,W,B,W,W,W,B,W,W,W,B,
	B,W,W,B,W,W,W,W,W,B,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,B,B,B,B,B,B,B,B,B,B,B,B,
};

static const unsigned int dt_icon_maximize_px[DT_ICON_W * DT_ICON_H] = {
	B,B,B,B,B,B,B,B,B,B,B,B,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,B,B,B,B,W,W,W,W,W,B,
	B,W,W,B,W,W,W,W,W,W,W,W,B,
	B,W,W,B,W,W,W,W,W,W,W,W,B,
	B,W,W,B,W,W,W,W,W,B,W,W,B,
	B,W,W,W,W,W,W,W,W,B,W,W,B,
	B,W,W,W,W,W,W,W,W,B,W,W,B,
	B,W,W,W,W,W,B,B,B,B,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,B,B,B,B,B,B,B,B,B,B,B,B,
};

static const unsigned int dt_icon_restore_px[DT_ICON_W * DT_ICON_H] = {
	B,B,B,B,B,B,B,B,B,B,B,B,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,W,W,B,W,W,W,W,W,W,B,
	B,W,W,W,W,B,W,W,W,W,W,W,B,
	B,W,W,W,W,B,W,W,W,W,W,W,B,
	B,W,B,B,B,B,W,W,W,W,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,W,W,W,W,B,B,B,B,W,B,
	B,W,W,W,W,W,W,B,W,W,W,W,B,
	B,W,W,W,W,W,W,B,W,W,W,W,B,
	B,W,W,W,W,W,W,B,W,W,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,B,B,B,B,B,B,B,B,B,B,B,B,
};

static const unsigned int dt_icon_minimize_px[DT_ICON_W * DT_ICON_H] = {
	B,B,B,B,B,B,B,B,B,B,B,B,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,B,B,B,B,B,B,B,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,B,
	B,B,B,B,B,B,B,B,B,B,B,B,B,
};

#undef T
#undef B
#undef W