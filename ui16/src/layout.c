/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: layout.c
 *
 */

#include "ui16.h"
#include "ui16_priv.h"

static int ui16__resolveSize(
    ui16_size_t node_size,
    int available_space,
    const ui16_renderer_t *renderer,
    ui16_node_t *node,
    int is_width_axis
) {
    if (node_size.kind == UI16_SIZE_PIXELS) return node_size.value;
    if (node_size.kind == UI16_SIZE_PERCENT) return (available_space * node_size.value) / 100;

    if (node_size.kind == UI16_SIZE_AUTOSIZE)
    {
        int measured_width = 0;
        int measured_height = 0;

        if (renderer && renderer->measureText && node->text) renderer->measureText(node->text, &measured_width, &measured_height);

        return is_width_axis ? measured_width : measured_height;
    }

    return available_space;
}

static void ui16__layoutChildren(ui16_node_t *parent_node, const ui16_renderer_t *renderer)
{
    int content_x = parent_node->box_x + parent_node->style.padding;
    int content_y = parent_node->box_y + parent_node->style.padding;
    int content_width = parent_node->box_width - parent_node->style.padding * 2;
    int content_height = parent_node->box_height - parent_node->style.padding * 2;

    if (content_width < 0) content_width = 0;
    if (content_height < 0) content_height = 0;

    int is_row_layout = (parent_node->style.layout == UI16_LAYOUT_ROW);
    int main_axis_total = is_row_layout ? content_width : content_height;

    int child_count = 0;
    int fill_count = 0;
    int fixed_total = 0;

    for (ui16_node_t *child_node = parent_node->first_child; child_node; child_node = child_node->next_sibling)
    {
        child_count++;

        ui16_size_t main_size = is_row_layout ? child_node->style.width : child_node->style.height;
        int main_available = is_row_layout ? content_width : content_height;

        if (main_size.kind == UI16_SIZE_FILL) fill_count++;
        else fixed_total += ui16__resolveSize(main_size, main_available, renderer, child_node, is_row_layout);
    }

    if (child_count > 1) fixed_total += parent_node->style.gap * (child_count - 1);

    int remaining_space = main_axis_total - fixed_total;
    if (remaining_space < 0) remaining_space = 0;

    int fill_share = (fill_count > 0) ? (remaining_space / fill_count) : 0;

    int running_offset = 0;

    for (ui16_node_t *child_node = parent_node->first_child; child_node; child_node = child_node->next_sibling)
    {
        ui16_size_t main_size = is_row_layout ? child_node->style.width : child_node->style.height;
        ui16_size_t cross_size = is_row_layout ? child_node->style.height : child_node->style.width;

        int main_available = is_row_layout ? content_width : content_height;
        int cross_available = is_row_layout ? content_height : content_width;

        int resolved_main = (main_size.kind == UI16_SIZE_FILL)
            ? fill_share
            : ui16__resolveSize(main_size, main_available, renderer, child_node, is_row_layout)
        ;

        int resolved_cross = ui16__resolveSize(cross_size, cross_available, renderer, child_node, !is_row_layout);

        if (is_row_layout)
        {
            child_node->box_x = content_x + running_offset;
            child_node->box_y = content_y;
            child_node->box_width = resolved_main;
            child_node->box_height = resolved_cross;
        }
        else
        {
            child_node->box_x = content_x;
            child_node->box_y = content_y + running_offset;
            child_node->box_width = resolved_cross;
            child_node->box_height = resolved_main;
        }

        running_offset += resolved_main + parent_node->style.gap;

        if (child_node->first_child) ui16__layoutChildren(child_node, renderer);
    }
}

void ui16__computeLayout(int screen_width ,int screen_height, const ui16_renderer_t *renderer)
{
    ui16_node_t *root_node = ui16__rootNode();
    if (!root_node) return;

    root_node->box_x = 0;
    root_node->box_y = 0;
    root_node->box_width = ui16__resolveSize(root_node->style.width, screen_width, renderer, root_node, 1);
    root_node->box_height = ui16__resolveSize(root_node->style.height, screen_height, renderer, root_node, 0);

    ui16__layoutChildren(root_node, renderer);
}