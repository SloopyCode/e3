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
#include "ui16_priv.h"

ui16_style_t ui16__defaultStyle(void)
{
    ui16_style_t default_style;

    default_style.width = autosize;
    default_style.height = autosize;
    default_style.min_width  = -1;
    default_style.max_width  = -1;
    default_style.min_height = -1;
    default_style.max_height = -1;
    default_style.background = 0;
    default_style.color = 0xFFFFFFFFu;
    default_style.padding = 0;
    default_style.margin = 0;
    default_style.gap = 0;
    default_style.radius = 0;
    default_style.border_width = 0;
    default_style.border_color = 0;
    default_style.layout = row;
    default_style.font = fontInherit;
    default_style.align_items = UI16_ALIGN_START;
    default_style.justify_content = UI16_JUSTIFY_START;
    default_style.wrap = 0;
    default_style.position = UI16_POSITION_STATIC;
    default_style.left = 0;
    default_style.top = 0;
    default_style.layer = 0;

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

            case UI16_MOD_MIN_WIDTH:
                result_style.min_width = current_mod->data.int_value;
                break;

            case UI16_MOD_MAX_WIDTH:
                result_style.max_width = current_mod->data.int_value;
                break;

            case UI16_MOD_MIN_HEIGHT:
                result_style.min_height = current_mod->data.int_value;
                break;

            case UI16_MOD_MAX_HEIGHT:
                result_style.max_height = current_mod->data.int_value;
                break;

            case UI16_MOD_BACKGROUND:
                result_style.background = current_mod->data.color_value;
                break;

            case UI16_MOD_COLOR:
                result_style.color = current_mod->data.color_value;
                break;

            case UI16_MOD_MARGIN:
                result_style.margin = current_mod->data.int_value;
                break;

            case UI16_MOD_PADDING:
                result_style.padding = current_mod->data.int_value;
                break;

            case UI16_MOD_GAP:
                result_style.gap = current_mod->data.int_value;
                break;

            case UI16_MOD_BORDER_WIDTH:
                result_style.border_width = current_mod->data.int_value;
                break;

            case UI16_MOD_BORDER_COLOR:
                result_style.border_color = current_mod->data.color_value;
                break;

            case UI16_MOD_RADIUS:
                result_style.radius = current_mod->data.int_value;
                break;

            case UI16_MOD_LAYOUT:
                result_style.layout = current_mod->data.layout_value;
                break;

            case UI16_MOD_FONT:
                result_style.font = current_mod->data.font_value;
                break;

            case UI16_MOD_ALIGN_ITEMS:
                result_style.align_items = current_mod->data.align_value;
                break;

            case UI16_MOD_JUSTIFY_CONTENT:
                result_style.justify_content = current_mod->data.justify_value;
                break;

            case UI16_MOD_WRAP:
                result_style.wrap = current_mod->data.int_value;
                break;

            case UI16_MOD_POSITION:
                result_style.position = current_mod->data.position_value;
                break;

            case UI16_MOD_LEFT:
                result_style.left = current_mod->data.int_value;
                break;

            case UI16_MOD_TOP:
                result_style.top = current_mod->data.int_value;
                break;

            case UI16_MOD_LAYER:
                result_style.layer = current_mod->data.int_value;
                break;

            default:
                break;

        }
    }

    return result_style;
}

ui16_font_t ui16__resolveFont(ui16_node_t *node)
{
    for (ui16_node_t *current_node = node; current_node; current_node = current_node->parent)
    {
        if (current_node->style.font.kind != UI16_FONT_INHERIT)
            return current_node->style.font;
    }

    return ui16__genericStyle().font;
}