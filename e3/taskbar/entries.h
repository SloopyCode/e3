/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: e3
 * FILE: entries.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#include "tb_widget.h"

tb_widget_t *entries_get(int *out_count);
void taskbar_load_entry_icon(int index);
