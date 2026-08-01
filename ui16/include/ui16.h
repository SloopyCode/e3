/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: ui16.h
 *
 */

#pragma once

typedef enum
{
    UI16_SIZE_PIXELS,
    UI16_SIZE_PERCENT,
    UI16_SIZE_FILL,
    UI16_SIZE_AUTOSIZE
} ui16_size_kind_t;

typedef struct
{
    ui16_size_kind_t kind;
    int  value;
} ui16_size_t;

typedef enum
{
    UI16_LAYOUT_ROW,
    UI16_LAYOUT_COLUMN
} ui16_layout_kind_t;

typedef enum
{
    UI16_FONT_INHERIT,
    UI16_FONT_REGULAR,
    UI16_FONT_BOLD
} ui16_font_kind_t;

typedef struct
{
    ui16_size_t width;
    ui16_size_t height;
    unsigned int background;
    int  padding;
    int  gap;
    int  radius;
    ui16_layout_kind_t layout;
    ui16_font_kind_t font;
} ui16_style_t;

typedef enum
{
    UI16_NODE_CONTAINER,
    UI16_NODE_BUTTON,
    UI16_NODE_LABEL
} ui16_node_kind_t;

typedef struct ui16_node_s
{
    ui16_node_kind_t kind;
    ui16_style_t style;
    const char *text;

    struct ui16_node_s *parent;
    struct ui16_node_s *first_child;
    struct ui16_node_s *last_child;
    struct ui16_node_s *next_sibling;

    int  box_x;
    int  box_y;
    int  box_width;
    int  box_height;
} ui16_node_t;
/*
typedef struct
{
    void (*drawRect)(int x, int y, int w, int h, unsigned int color, int radius);
    void (*drawText)(int x, int y, const char *text, unsigned int color);
    void (*measureText)(const char *text, int *out_width, int *out_height);
} ui16_renderer_t;*/

typedef enum
{
    UI16_MOD_WIDTH,
    UI16_MOD_HEIGHT,
    UI16_MOD_BACKGROUND,
    UI16_MOD_PADDING,
    UI16_MOD_GAP,
    UI16_MOD_RADIUS,
    UI16_MOD_LAYOUT,
    UI16_MOD_FONT,

    UI16_MOD_END
} ui16_style_mod_kind_t;

typedef struct
{
    ui16_style_mod_kind_t kind;
    union
    {
        ui16_size_t size_value;
        unsigned int color_value;
        int  int_value;

        ui16_layout_kind_t layout_value;
        ui16_font_kind_t font_value;

    } data;
} ui16_style_mod_t;

ui16_style_t ui16__defaultStyle(void);
ui16_style_t ui16__applyMods(ui16_style_t base_style, const ui16_style_mod_t *mod_list);

#define style(...) \
    ui16__applyMods(ui16__defaultStyle(), \
        (const ui16_style_mod_t[])        \
        {                                 \
            __VA_ARGS__,{                 \
                UI16_MOD_END, { 0 }       \
            }                             \
        }                                 \
    )

// styles
#define width(size_arg)    ((ui16_style_mod_t){ UI16_MOD_WIDTH,      .data.size_value = (size_arg) })
#define height(size_arg)   ((ui16_style_mod_t){ UI16_MOD_HEIGHT,     .data.size_value = (size_arg) })
#define bg(color_arg)      ((ui16_style_mod_t){ UI16_MOD_BACKGROUND, .data.color_value = (color_arg) })
#define padding(int_arg)   ((ui16_style_mod_t){ UI16_MOD_PADDING,    .data.int_value = (int_arg) })
#define gap(int_arg)       ((ui16_style_mod_t){ UI16_MOD_GAP,        .data.int_value = (int_arg) })
#define radius(int_arg)    ((ui16_style_mod_t){ UI16_MOD_RADIUS,     .data.int_value = (int_arg) })
#define layout(layout_arg) ((ui16_style_mod_t){ UI16_MOD_LAYOUT,     .data.layout_value = (layout_arg) })
#define font(font_arg)     ((ui16_style_mod_t){ UI16_MOD_FONT,       .data.font_value = (font_arg) })

// sizes
#define px(pixel_amount)   ((ui16_size_t){ UI16_SIZE_PIXELS, (pixel_amount) })
#define percent(percent_amount) ((ui16_size_t){ UI16_SIZE_PERCENT, (percent_amount) })
#define fill     ((ui16_size_t){ UI16_SIZE_FILL, 0 })
#define autosize ((ui16_size_t){ UI16_SIZE_AUTOSIZE, 0 })

#define row    UI16_LAYOUT_ROW
#define column UI16_LAYOUT_COLUMN
#define fontInherit   UI16_FONT_INHERIT
#define fontRegular   UI16_FONT_REGULAR
#define fontBold      UI16_FONT_BOLD


#define rgb(             \
	        red_value,   \
			green_value, \
			blue_value   \
        ) (\
            0xFF000000u |\
            ((unsigned int)(red_value) << 16)  |\
            ((unsigned int)(green_value) << 8) |\
            (unsigned int)(blue_value)          \
        )

ui16_node_t *ui16__setRootStyled(ui16_style_t root_style, unsigned int *target_buffer, int buffer_width, int buffer_height);
ui16_node_t *ui16__setRootDefault(unsigned int *target_buffer, int buffer_width, int buffer_height);

#define UI16_ARG4(first_arg, second_arg, third_arg, fourth_arg, name_arg, ...) name_arg

#define ui16_setRoot(...) \
    UI16_ARG4(\
        __VA_ARGS__,                        \
        ui16__setRootStyled,                \
        ui16__setRootDefault)(__VA_ARGS__)

void ui16_frame(void);

ui16_node_t *ui16__containerBegin(ui16_style_t container_style);
void ui16__containerEnd(void);

#define ui16_container(style_arg) \
    for (ui16_node_t *ui16_loop_node = ui16__containerBegin(style_arg); \
         ui16_loop_node;     \
         ui16_loop_node =    \
         (ui16__containerEnd(), (ui16_node_t *)0))

ui16_node_t *ui16__labelSimple(const char *label_text);
ui16_node_t *ui16__labelStyled(ui16_style_t label_style, const char *label_text);

#define UI16_ARG2(first_arg, second_arg, name_arg, ...) name_arg

#define ui16_label(...) UI16_ARG2(\
	                              __VA_ARGS__, \
								  ui16__labelStyled, \
								  ui16__labelSimple) \
                        (__VA_ARGS__)