/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: e3
 * FILE: comp.c
 * CREATED BY: emex
 * MODIFIED BY: Offihito
 *
 */

#include "comp.h"
#include <sys/fb.h>
#include <unistd.h>
#include <stdio.h>
#include "../config/cfg.h"
#include <stdlib.h>
#include <string.h>

static int g_fd = -1;
static int g_w = 0;
static int g_h = 0;
static unsigned int *g_buf = 0;
static unsigned int g_stride = 0;
static unsigned int *g_buf_shadow = 0;

static int g_real_fd = -1;
static int g_physical_width = 0;
static int g_physical_height = 0;
static unsigned int g_physical_stride = 0;
static unsigned int *g_real_fb_buffer = 0;
static unsigned int *g_internal_render_surface = 0;

// fixed by @offihito

static void check_g_buf(const char *where)
{
    if (g_buf_shadow && g_buf != g_buf_shadow)
    {
        printf(
        	"[COMP] !!! g_buf CHANGED at %s: was=%p now=%p\n",
            where,
            (void*)g_buf_shadow,
            (void*)g_buf
        );
    }
    g_buf_shadow = g_buf;
}

void comp_init(int framebuffer_file_descriptor, int requested_internal_width, int requested_internal_height)
{
	printf(":: comp: starting...\n");

    fb_info_t framebuffer_information_structure;
    unsigned long mapped_virtual_address;

    g_fd = framebuffer_file_descriptor;
    g_real_fd = framebuffer_file_descriptor;

    if (ioctl(g_real_fd, FB_IOCTL_GET_INFO, &framebuffer_information_structure) < 0)
    {
        return;
    }

    g_physical_width = (int)framebuffer_information_structure.width;
    g_physical_height = (int)framebuffer_information_structure.height;
    g_physical_stride = framebuffer_information_structure.pitch / 4;

    mapped_virtual_address = 0;
    if (ioctl(g_real_fd, FB_IOCTL_MAP, &mapped_virtual_address) < 0)
    {
        return;
    }

    g_real_fb_buffer = (unsigned int *)mapped_virtual_address;

    #if RENDERER_SCALING_ENABLED
    	printf(":: comp: rendered scaling is enabled!\n");
	    g_w = requested_internal_width;
	    g_h = requested_internal_height;
	    g_stride = (unsigned int)requested_internal_width;

	    if (g_internal_render_surface)
	    {
	        free(g_internal_render_surface);
	    }
		g_internal_render_surface = (unsigned int *)malloc(
    		(unsigned int)(g_w * g_h * sizeof(unsigned int))
		);

		if (!g_internal_render_surface)
		{
		    printf(
				":: comp: internal_render_surface allocation failed\n"
				"   we will fall back to real framebuffer :(\n"
			);

		    g_w = g_physical_width;
		    g_h = g_physical_height;
		    g_stride = g_physical_stride;
		    g_buf = g_real_fb_buffer;

		    return;
		}

        if (g_internal_render_surface)
        {
		    memset(
				g_internal_render_surface,
				0,
				(unsigned int)(g_w * g_h * sizeof(unsigned int))
		    );
        }

	    g_buf = g_internal_render_surface;
    #else
	    g_w = g_physical_width;
	    g_h = g_physical_height;
	    g_stride = g_physical_stride;
	    g_buf = g_real_fb_buffer;
    #endif

    g_buf_shadow = g_buf;
}

void comp_capture(void)
{
	check_g_buf(__func__);
}

void comp_fill(
	int x,
	int y,
	int w,
	int h,
	unsigned int color
){
	check_g_buf(__func__);
    if (!g_buf) return;
    for (int dy = 0; dy < h; dy++)
    {
        int py = y + dy;
        if (py < 0 || py >= g_h) continue;
        unsigned int *row = g_buf + (unsigned)py * g_stride;

        int x0 = x < 0 ? 0 : x;
        int x1 = (x + w) > g_w ? g_w : (x + w);
        for (int px = x0; px < x1; px++) row[px] = color;
    }
}

unsigned int comp_get(
	int x,
	int y
){
	check_g_buf(__func__);
    if (
    	!g_buf ||
     	x < 0 ||
      	x >= g_w ||
       	y < 0 ||
        y >= g_h
    ) return 0;

    return g_buf[(unsigned)y * g_stride + (unsigned)x];
}

void comp_set(
	int x,
	int y,
	unsigned int c
) {
	check_g_buf(__func__);
    if (
    	!g_buf ||
     	x < 0 ||
      	x >= g_w ||
       	y < 0 ||
        y >= g_h
    ) return;

    g_buf[(unsigned)y * g_stride + (unsigned)x] = c;
}

void comp_put_row(
	int x,
	int y,
	const unsigned int *row,
	int len
) {
	check_g_buf(__func__);
    if (
    	!g_buf ||
     	y < 0 ||
      	y >= g_h
    ) return;

    unsigned int *dst = g_buf + (unsigned)y * g_stride;

    for (int i = 0; i < len; i++)
    {
        int px = x + i;
        if (px >= 0 && px < g_w) dst[px] = row[i];
    }
}

void
comp_copy_rect(
	int src_x,
	int src_y,
	int dst_x,
	int dst_y,
	int w,
	int h
){
	check_g_buf(__func__);
    if (
    	!g_buf ||
     	w <= 0 ||
        h <= 0
    ) return;

    int ystep = (dst_y <= src_y) ? 1 : -1;
    int r0 = (ystep == 1) ? 0 : h - 1;
    int r1 = (ystep == 1) ? h : -1;

    for (int r = r0; r != r1; r += ystep)
    {
        int sy = src_y + r, dy = dst_y + r;
        if (
        	sy < 0 ||
         	sy >= g_h ||
          	dy < 0 ||
           	dy >= g_h
        ) continue;

        unsigned int *srow = g_buf + (unsigned)sy * g_stride;
        unsigned int *drow = g_buf + (unsigned)dy * g_stride;

        int xstep = (dst_x <= src_x) ? 1 : -1;
        int c0 = (xstep == 1) ? 0 : w - 1;
        int c1 = (xstep == 1) ? w : -1;

        for (int c = c0; c != c1; c += xstep)
        {
            int spx = src_x + c, dpx = dst_x + c;
            if (spx >= 0 && spx < g_w && dpx >= 0 && dpx < g_w) drow[dpx] = srow[spx];
        }
    }
}

void comp_put_pixels(
	int x,
	int y,
	int w,
	int h,
	const unsigned int *pixels
) {
	check_g_buf(__func__);
    if (!g_buf || !pixels) return;
    for (int row = 0; row < h; row++)
    {
        int py = y + row;
        if (py < 0 || py >= g_h) continue;
        unsigned int *dst = g_buf + (unsigned)py * g_stride;

        for (int col = 0; col < w; col++)
        {
            int px = x + col;
            if (px >= 0 && px < g_w) dst[px] = pixels[row * w + col];
        }
    }
}

static void bilinear_scale_framebuffer_pixels(
    const unsigned int *source_pixels_pointer,
    int source_width_pixels,
    int source_height_pixels,
    unsigned int *destination_pixels_pointer,
    int destination_width_pixels,
    int destination_height_pixels,
    unsigned int destination_stride_pixels
)
{
	// i think its better if i do more detailed variable names... even tho its shitty to type then... but otherwise im too dumb for this math TwT
    int x;
    int y;
    float horizontal_ratio_factor;
    float vertical_ratio_factor;

    /*standard math stuff to blend pixels together
     * so it doesn't look like we're playing Doom on a fkin toaster */
    if (source_width_pixels <= 0 || source_height_pixels <= 0 || destination_width_pixels <= 0 || destination_height_pixels <= 0)
    {
        return;
    }

    horizontal_ratio_factor = (float)source_width_pixels / (float)destination_width_pixels;
    vertical_ratio_factor = (float)source_height_pixels / (float)destination_height_pixels;

    for (y = 0; y < destination_height_pixels; y++)
    {
        float source_y_coordinate = ((float)y + 0.5f) * vertical_ratio_factor - 0.5f;
        if (source_y_coordinate < 0.0f)
        {
            source_y_coordinate = 0.0f;
        }
        if (source_y_coordinate > (float)(source_height_pixels - 1))
        {
            source_y_coordinate = (float)(source_height_pixels - 1);
        }

        for (x = 0; x < destination_width_pixels; x++)
        {
            float source_x_coordinate = ((float)x + 0.5f) * horizontal_ratio_factor - 0.5f;
            if (source_x_coordinate < 0.0f)
            {
                source_x_coordinate = 0.0f;
            }
            if (source_x_coordinate > (float)(source_width_pixels - 1))
            {
                source_x_coordinate = (float)(source_width_pixels - 1);
            }

            int source_left_index = (int)source_x_coordinate;
            int source_top_index = (int)source_y_coordinate;

            int source_right_index = source_left_index + 1;
            if (source_right_index >= source_width_pixels)
            {
                source_right_index = source_width_pixels - 1;
            }

            int source_bottom_index = source_top_index + 1;
            if (source_bottom_index >= source_height_pixels)
            {
                source_bottom_index = source_height_pixels - 1;
            }

            float horizontal_weight_fraction = source_x_coordinate - (float)source_left_index;
            float vertical_weight_fraction = source_y_coordinate - (float)source_top_index;

            float opposite_horizontal_weight = 1.0f - horizontal_weight_fraction;
            float opposite_vertical_weight = 1.0f - vertical_weight_fraction;

            float top_left_weight = opposite_horizontal_weight * opposite_vertical_weight;
            float top_right_weight = horizontal_weight_fraction * opposite_vertical_weight;
            float bottom_left_weight = opposite_horizontal_weight * vertical_weight_fraction;
            float bottom_right_weight = horizontal_weight_fraction * vertical_weight_fraction;

            unsigned int top_left_pixel = source_pixels_pointer[source_top_index * source_width_pixels + source_left_index];
            unsigned int top_right_pixel = source_pixels_pointer[source_top_index * source_width_pixels + source_right_index];
            unsigned int bottom_left_pixel = source_pixels_pointer[source_bottom_index * source_width_pixels + source_left_index];
            unsigned int bottom_right_pixel = source_pixels_pointer[source_bottom_index * source_width_pixels + source_right_index];

            unsigned int top_left_alpha = (top_left_pixel >> 24) & 0xFF;
            unsigned int top_left_red   = (top_left_pixel >> 16) & 0xFF;
            unsigned int top_left_green = (top_left_pixel >> 8) & 0xFF;
            unsigned int top_left_blue  = top_left_pixel & 0xFF;

            unsigned int top_right_alpha = (top_right_pixel >> 24) & 0xFF;
            unsigned int top_right_red   = (top_right_pixel >> 16) & 0xFF;
            unsigned int top_right_green = (top_right_pixel >> 8) & 0xFF;
            unsigned int top_right_blue  = top_right_pixel & 0xFF;

            unsigned int bottom_left_alpha = (bottom_left_pixel >> 24) & 0xFF;
            unsigned int bottom_left_red   = (bottom_left_pixel >> 16) & 0xFF;
            unsigned int bottom_left_green = (bottom_left_pixel >> 8) & 0xFF;
            unsigned int bottom_left_blue  = bottom_left_pixel & 0xFF;

            unsigned int bottom_right_alpha = (bottom_right_pixel >> 24) & 0xFF;
            unsigned int bottom_right_red   = (bottom_right_pixel >> 16) & 0xFF;
            unsigned int bottom_right_green = (bottom_right_pixel >> 8) & 0xFF;
            unsigned int bottom_right_blue  = bottom_right_pixel & 0xFF;c

            unsigned int interpolated_alpha = (unsigned int)(
                (float)top_left_alpha * top_left_weight +
                (float)top_right_alpha * top_right_weight +
                (float)bottom_left_alpha * bottom_left_weight +
                (float)bottom_right_alpha * bottom_right_weight +
                0.5f
            );
            unsigned int interpolated_red = (unsigned int)(
                (float)top_left_red * top_left_weight +
                (float)top_right_red * top_right_weight +
                (float)bottom_left_red * bottom_left_weight +
                (float)bottom_right_red * bottom_right_weight +
                0.5f
            );
            unsigned int interpolated_green = (unsigned int)(
                (float)top_left_green * top_left_weight +
                (float)top_right_green * top_right_weight +
                (float)bottom_left_green * bottom_left_weight +
                (float)bottom_right_green * bottom_right_weight +
                0.5f
            );
            unsigned int interpolated_blue = (unsigned int)(
                (float)top_left_blue * top_left_weight +
                (float)top_right_blue * top_right_weight +
                (float)bottom_left_blue * bottom_left_weight +
                (float)bottom_right_blue * bottom_right_weight +
                0.5f
            );

            destination_pixels_pointer[
                (unsigned int)y * destination_stride_pixels + (unsigned int)x
            ] =
                (interpolated_alpha << 24) |
                (interpolated_red << 16) |
                (interpolated_green << 8) |
                interpolated_blue;
        }
    }
}

void comp_flush(void)
{
    check_g_buf(__func__);
    if (!g_buf || g_real_fd < 0)
    {
        return;
    }

    #if RENDERER_SCALING_ENABLED
	    if (g_internal_render_surface && g_real_fb_buffer)
	    {
			bilinear_scale_framebuffer_pixels(
			    g_internal_render_surface,
			    g_w,
			    g_h,
			    g_real_fb_buffer,
			    g_physical_width,
			    g_physical_height,
			    g_physical_stride
			);
	    }
    #endif

    ioctl(g_real_fd, FB_IOCTL_FLUSH, 0);
}

int comp_w(void) {
    if (g_w <= 0 || g_h <= 0)
        printf("[COMP] !!! comp_w() called with corrupted state: g_w=%d g_h=%d g_buf=%p g_fd=%d\n",
               g_w, g_h, (void*)g_buf, g_fd);
    return g_w;
}

int comp_h(void) {
    if (g_w <= 0 || g_h <= 0)
        printf("[COMP] !!! comp_h() called with corrupted state: g_w=%d g_h=%d g_buf=%p g_fd=%d\n",
               g_w, g_h, (void*)g_buf, g_fd);
    return g_h;
}