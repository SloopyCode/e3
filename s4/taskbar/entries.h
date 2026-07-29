/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: entries.h
 *
 */

#pragma once

#include "tb_widget.h"

tb_widget_t *entries_get(int *out_count);
void taskbar_load_entry_icon(int index);
