/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: style.c
 *
 */

#include "ui16.h"

ui16_style_t ui16__defaultStyle(void)
{
    ui16_style_t default_style;

    default_style.width = autosize;
    default_style.height = autosize;
    default_style.background = 0;
    default_style.padding = 0;
    default_style.gap = 0;
    default_style.radius = 0;
    default_style.layout = row;

    return  default_style;
}

ui16_style_t ui16__applyMods(ui16_style_t base_style, const ui16_style_mod_t *mod_list)
{
    ui16_style_t result_style = base_style;

    for (
        int mod_index = 0;
        mod_list[mod_index].kind != UI16_MOD_END;
        mod_index++
    ){
        const ui16_style_mod_t *current_mod = &mod_list[mod_index];

        switch (current_mod->kind)
        {
            case UI16_MOD_WIDTH:
                result_style.width = current_mod->data.size_value;
                break;

            case UI16_MOD_HEIGHT:
                result_style.height = current_mod->data.size_value;
                break;

            case UI16_MOD_BACKGROUND:
                result_style.background = current_mod->data.color_value;
                break;

            case UI16_MOD_PADDING:
                result_style.padding = current_mod->data.int_value;
                break;

            case UI16_MOD_GAP:
                result_style.gap = current_mod->data.int_value;
                break;

            case UI16_MOD_RADIUS:
                result_style.radius = current_mod->data.int_value;
                break;

            case UI16_MOD_LAYOUT:
                result_style.layout = current_mod->data.layout_value;
                break;

            default:
                break;

        }
    }

    return result_style;
}