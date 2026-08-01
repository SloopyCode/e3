/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: tree.c
 *
 */

#include "ui16.h"
#include "ui16_priv.h"

#define UI16_ARENA_BYTES 65536

static unsigned char arena_buffer[UI16_ARENA_BYTES];
static unsigned long arena_used = 0;
static ui16_node_t *root_node = 0;
static ui16_node_t *current_parent = 0;

void *ui16__alloc(unsigned long byte_count)
{
    unsigned long aligned_count = (byte_count + sizeof(void *) - 1) & ~(sizeof(void *) - 1);

    if (arena_used + aligned_count > UI16_ARENA_BYTES) return 0;

    void *allocated_pointer = arena_buffer + arena_used;
    arena_used += aligned_count;

    return allocated_pointer;
}

ui16_node_t *ui16__rootNode(void)
{
    return root_node;
}

ui16_node_t *ui16__currentParent(void)
{
    return current_parent;
}

void ui16__setCurrentParent(ui16_node_t *parent_node)
{
    current_parent = parent_node;
}

ui16_node_t *ui16__attachNode(ui16_node_kind_t node_kind, ui16_style_t node_style, const char *node_text)
{
    ui16_node_t *new_node = (ui16_node_t *)ui16__alloc(sizeof(ui16_node_t));
    if (!new_node) return 0;

    new_node->kind = node_kind;
    new_node->style = node_style;
    new_node->text = node_text;
    new_node->parent = current_parent;
    new_node->first_child = 0;
    new_node->last_child = 0;
    new_node->next_sibling = 0;
    new_node->box_x = 0;
    new_node->box_y = 0;
    new_node->box_width = 0;
    new_node->box_height = 0;

    if (current_parent)
    {
        if (current_parent->last_child) current_parent->last_child->next_sibling = new_node;
        else current_parent->first_child = new_node;

        current_parent->last_child = new_node;
    }

    return new_node;
}

ui16_node_t *ui16__setRootStyled(ui16_style_t root_style, unsigned int *target_buffer, int buffer_width, int buffer_height)
{
    arena_used = 0;
    root_node = 0;
    current_parent = 0;

    ui16__setTargetBuffer(target_buffer, buffer_width, buffer_height);

    root_node = (ui16_node_t *)ui16__alloc(sizeof(ui16_node_t));
    if (!root_node) return 0;

    root_node->kind = UI16_NODE_CONTAINER;
    root_node->style = root_style;
    root_node->text = 0;
    root_node->parent = 0;
    root_node->first_child = 0;
    root_node->last_child = 0;
    root_node->next_sibling = 0;
    root_node->box_x = 0;
    root_node->box_y = 0;
    root_node->box_width = 0;
    root_node->box_height = 0;

    current_parent = root_node;

    return root_node;
}

ui16_node_t *ui16__setRootDefault(unsigned int *target_buffer, int buffer_width, int buffer_height)
{
    return ui16__setRootStyled(ui16__genericStyle(), target_buffer, buffer_width, buffer_height);
}