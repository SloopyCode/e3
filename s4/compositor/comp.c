/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: comp.c
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

static void bilinear_scale_framebuffer_region(
    const unsigned int *source_pixels_pointer,
    int source_width_pixels,
    int source_height_pixels,
    unsigned int *destination_pixels_pointer,
    int destination_width_pixels,
    int destination_height_pixels,
    unsigned int destination_stride_pixels,
    int destination_x0,
    int destination_y0,
    int destination_x1,
    int destination_y1
)
{
    int x;
    int y;
    int fixed_point_horizontal_ratio;
    int fixed_point_vertical_ratio;

    if (source_width_pixels <= 0 || source_height_pixels <= 0 || destination_width_pixels <= 0 || destination_height_pixels <= 0)
    {
        return;
    }

    fixed_point_horizontal_ratio = (int)(((float)source_width_pixels / (float)destination_width_pixels) * 65536.0f);
    fixed_point_vertical_ratio = (int)(((float)source_height_pixels / (float)destination_height_pixels) * 65536.0f);

    for (y = destination_y0; y < destination_y1; y++)
    {
        int fixed_point_source_y = y * fixed_point_vertical_ratio + (fixed_point_vertical_ratio / 2) - 32768;
        int source_top_index;
        int source_bottom_index;
        int vertical_weight_fraction;
        int opposite_vertical_weight;

        if (fixed_point_source_y < 0)
        {
            fixed_point_source_y = 0;
        }
        if (fixed_point_source_y > (source_height_pixels - 1) * 65536)
        {
            fixed_point_source_y = (source_height_pixels - 1) * 65536;
        }

        source_top_index = fixed_point_source_y >> 16;
        source_bottom_index = source_top_index + 1;
        if (source_bottom_index >= source_height_pixels)
        {
            source_bottom_index = source_height_pixels - 1;
        }

        vertical_weight_fraction = (fixed_point_source_y >> 8) & 0xFF;
        opposite_vertical_weight = 256 - vertical_weight_fraction;

        for (x = destination_x0; x < destination_x1; x++)
        {
            int fixed_point_source_x = x * fixed_point_horizontal_ratio + (fixed_point_horizontal_ratio / 2) - 32768;
            int source_left_index;
            int source_right_index;
            int horizontal_weight_fraction;
            int opposite_horizontal_weight;

            int top_left_weight;
            int top_right_weight;
            int bottom_left_weight;
            int bottom_right_weight;

            unsigned int top_left_pixel;
            unsigned int top_right_pixel;
            unsigned int bottom_left_pixel;
            unsigned int bottom_right_pixel;

            unsigned int top_left_alpha;
            unsigned int top_left_red;
            unsigned int top_left_green;
            unsigned int top_left_blue;

            unsigned int top_right_alpha;
            unsigned int top_right_red;
            unsigned int top_right_green;
            unsigned int top_right_blue;

            unsigned int bottom_left_alpha;
            unsigned int bottom_left_red;
            unsigned int bottom_left_green;
            unsigned int bottom_left_blue;

            unsigned int bottom_right_alpha;
            unsigned int bottom_right_red;
            unsigned int bottom_right_green;
            unsigned int bottom_right_blue;

            unsigned int interpolated_alpha;
            unsigned int interpolated_red;
            unsigned int interpolated_green;
            unsigned int interpolated_blue;

            if (fixed_point_source_x < 0)
            {
                fixed_point_source_x = 0;
            }
            if (fixed_point_source_x > (source_width_pixels - 1) * 65536)
            {
                fixed_point_source_x = (source_width_pixels - 1) * 65536;
            }

            source_left_index = fixed_point_source_x >> 16;
            source_right_index = source_left_index + 1;
            if (source_right_index >= source_width_pixels)
            {
                source_right_index = source_width_pixels - 1;
            }

            horizontal_weight_fraction = (fixed_point_source_x >> 8) & 0xFF;
            opposite_horizontal_weight = 256 - horizontal_weight_fraction;

            top_left_weight = (opposite_horizontal_weight * opposite_vertical_weight) >> 8;
            top_right_weight = (horizontal_weight_fraction * opposite_vertical_weight) >> 8;
            bottom_left_weight = (opposite_horizontal_weight * vertical_weight_fraction) >> 8;
            bottom_right_weight = (horizontal_weight_fraction * vertical_weight_fraction) >> 8;

            top_left_pixel = source_pixels_pointer[source_top_index * source_width_pixels + source_left_index];
            top_right_pixel = source_pixels_pointer[source_top_index * source_width_pixels + source_right_index];
            bottom_left_pixel = source_pixels_pointer[source_bottom_index * source_width_pixels + source_left_index];
            bottom_right_pixel = source_pixels_pointer[source_bottom_index * source_width_pixels + source_right_index];

            top_left_alpha = (top_left_pixel >> 24) & 0xFF;
            top_left_red   = (top_left_pixel >> 16) & 0xFF;
            top_left_green = (top_left_pixel >> 8) & 0xFF;
            top_left_blue  = top_left_pixel & 0xFF;

            top_right_alpha = (top_right_pixel >> 24) & 0xFF;
            top_right_red   = (top_right_pixel >> 16) & 0xFF;
            top_right_green = (top_right_pixel >> 8) & 0xFF;
            top_right_blue  = top_right_pixel & 0xFF;

            bottom_left_alpha = (bottom_left_pixel >> 24) & 0xFF;
            bottom_left_red   = (bottom_left_pixel >> 16) & 0xFF;
            bottom_left_green = (bottom_left_pixel >> 8) & 0xFF;
            bottom_left_blue  = bottom_left_pixel & 0xFF;

            bottom_right_alpha = (bottom_right_pixel >> 24) & 0xFF;
            bottom_right_red   = (bottom_right_pixel >> 16) & 0xFF;
            bottom_right_green = (bottom_right_pixel >> 8) & 0xFF;
            bottom_right_blue  = bottom_right_pixel & 0xFF;

            interpolated_alpha = (
                top_left_alpha * top_left_weight +
                top_right_alpha * top_right_weight +
                bottom_left_alpha * bottom_left_weight +
                bottom_right_alpha * bottom_right_weight
            ) >> 8;
            interpolated_red = (
                top_left_red * top_left_weight +
                top_right_red * top_right_weight +
                bottom_left_red * bottom_left_weight +
                bottom_right_red * bottom_right_weight
            ) >> 8;
            interpolated_green = (
                top_left_green * top_left_weight +
                top_right_green * top_right_weight +
                bottom_left_green * bottom_left_weight +
                bottom_right_green * bottom_right_weight
            ) >> 8;
            interpolated_blue = (
                top_left_blue * top_left_weight +
                top_right_blue * top_right_weight +
                bottom_left_blue * bottom_left_weight +
                bottom_right_blue * bottom_right_weight
            ) >> 8;

            destination_pixels_pointer[y * destination_stride_pixels + x] =
                (interpolated_alpha << 24) |
                (interpolated_red << 16) |
                (interpolated_green << 8) |
                interpolated_blue;
        }
    }
}

void comp_flush_rect(int internal_x, int internal_y, int internal_width, int internal_height)
{
    check_g_buf(__func__);
    if (!g_buf || g_real_fd < 0)
    {
        return;
    }

    #if RENDERER_SCALING_ENABLED
	    if (g_internal_render_surface && g_real_fb_buffer)
	    {
	        int physical_x0;
	        int physical_y0;
	        int physical_x1;
	        int physical_y1;
	        fb_rect_t physical_flush_rectangle;

	        physical_x0 = internal_x * g_physical_width / g_w;
	        physical_y0 = internal_y * g_physical_height / g_h;
	        physical_x1 = (internal_x + internal_width) * g_physical_width / g_w + 1;
	        physical_y1 = (internal_y + internal_height) * g_physical_height / g_h + 1;

	        if (physical_x0 < 0)
			{
	            physical_x0 = 0;
	        }
	        if (physical_y0 < 0)
	        {
	            physical_y0 = 0;
	        }
	        if (physical_x1 > g_physical_width)
	        {
	            physical_x1 = g_physical_width;
	        }
	        if (physical_y1 > g_physical_height)
	        {
	            physical_y1 = g_physical_height;
	        }

	        bilinear_scale_framebuffer_region(
	            g_internal_render_surface,
	            g_w,
	            g_h,
	            g_real_fb_buffer,
	            g_physical_width,
	            g_physical_height,
	            g_physical_stride,
	            physical_x0,
	            physical_y0,
	            physical_x1,
	            physical_y1
	        );

	        physical_flush_rectangle.x = (unsigned int)physical_x0;
	        physical_flush_rectangle.y = (unsigned int)physical_y0;
	        physical_flush_rectangle.width  = (unsigned int)(physical_x1 - physical_x0);
	        physical_flush_rectangle.height = (unsigned int)(physical_y1 - physical_y0);

	        ioctl(g_real_fd, FB_IOCTL_FLUSH_RECT, &physical_flush_rectangle);
	        return;
	    }
    #else
	    {
	        fb_rect_t physical_flush_rectangle;
	        physical_flush_rectangle.x = (unsigned int)internal_x;
	        physical_flush_rectangle.y = (unsigned int)internal_y;
	        physical_flush_rectangle.width  = (unsigned int)internal_width;
	        physical_flush_rectangle.height = (unsigned int)internal_height;
	        ioctl(g_real_fd, FB_IOCTL_FLUSH_RECT, &physical_flush_rectangle);
	        return;
	    }
    #endif

    ioctl(g_real_fd, FB_IOCTL_FLUSH, 0);
}

void comp_flush(void)
{
    comp_flush_rect(0, 0, g_w, g_h);
}

int comp_w(void) {
    if (g_w <= 0 || g_h <= 0)
        printf(
        	"[COMP] !!! comp_w() called with corrupted state: g_w=%d g_h=%d g_buf=%p g_fd=%d\n",
            g_w, g_h, (void*)g_buf, g_fd
        );
    return g_w;
}

int comp_h(void) {
    if (g_w <= 0 || g_h <= 0)
        printf(
        	"[COMP] !!! comp_h() called with corrupted state: g_w=%d g_h=%d g_buf=%p g_fd=%d\n",
            g_w, g_h, (void*)g_buf, g_fd
        );
    return g_h;
}