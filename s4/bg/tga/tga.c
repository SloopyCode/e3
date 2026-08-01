/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: tga.c
 *
 */

#include "tga.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


typedef struct
{
    unsigned char    id_length;
    unsigned char    color_map_type;
    unsigned char    image_type;

    unsigned char    cmap_entry_size;

    unsigned char    pixel_depth;
    unsigned char    image_descriptor;

    unsigned short cmap_first_entry;
    unsigned short cmap_len;

    unsigned short x_origin;
    unsigned short y_origin;
    unsigned short width;
    unsigned short height;
} __attribute__((packed)) _tga_hdr_t;

static int _read_exact(int fd, void *buf, int n)
{
    unsigned char *p = (unsigned char *)buf;
    int remaining = n;

    while (remaining > 0)
    {
        int r = (int)read(fd, p, (size_t)remaining);
        if (r <= 0) return -1;

        p += r;
        remaining -= r;
    }
    return 0;
}

static int _skip(int fd, int n)
{
    unsigned char tmp[64];

    while (n > 0)
    {
        int chunk = (n > 64) ? 64 : n;
        int r = (int)read(fd, tmp, (size_t)chunk);
        if (r <= 0) return -1;
        n -= r;
    }
    return 0;
}

static void _store_pixel(
    bmp_image_t *img,
    int w,
    int h,
    int scan_index,
    const unsigned char *px, /* B,G,R[,A] */
    int bpp,
    int flip_v,
    int flip_h
) {
    int row = scan_index / w;
    int col = scan_index % w;

    int dst_row = flip_v ? (h - 1 - row) : row;
    int dst_col = flip_h ? (w - 1 - col) : col;

    unsigned int b = px[0];
    unsigned int g = px[1];
    unsigned int r = px[2];
    unsigned int a = (bpp == 4) ? px[3] : 0xFFu;

    img->pixels[dst_row * w + dst_col] = (a << 24) | (r << 16) | (g << 8) | b;
}

int tga_load(const char *path, bmp_image_t *img)
{
    if (!path || !img) return -1;

    img->pixels = NULL;
    img->width  = 0;
    img->height = 0;
    int w;
    int h;

    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;

    _tga_hdr_t hdr;
    if (_read_exact(fd, &hdr, (int)sizeof(hdr)) != 0) goto fail;

    if (hdr.color_map_type != 0) goto fail;

    int is_rle;
    if (hdr.image_type == 2) is_rle = 0;
    else if (hdr.image_type == 10) is_rle = 1;
    else goto fail;

    w = (int)hdr.width;
    h = (int)hdr.height;

    if (hdr.pixel_depth != 24 && hdr.pixel_depth != 32) goto fail;
    if (hdr.id_length > 0 && _skip(fd, hdr.id_length) != 0) goto fail;
    if (w <= 0 || h <= 0) goto fail;

    int bpp = hdr.pixel_depth / 8;

    img->pixels = (unsigned int *)malloc((size_t)(w * h) * 4);
    img->width  = w;
    img->height = h;

    if (!img->pixels) goto fail;


    int flip_v = !(hdr.image_descriptor & 0x20);
    int flip_h = (hdr.image_descriptor & 0x10) != 0;

    int total = w * h;
    int written = 0;

    unsigned char px[4];

    while (written < total)
    {
        if (is_rle)
        {
            unsigned char packet_hdr;
            if (_read_exact(fd, &packet_hdr, 1) != 0) goto fail2;

            int count = (packet_hdr & 0x7F) + 1;

            if (packet_hdr & 0x80)
            {
                if (_read_exact(fd, px, bpp) != 0) goto fail2;

                for (
                	int i = 0;
                 	i < count && written < total;
                  	i++,
                   	written++
                ) _store_pixel(img, w, h, written, px, bpp, flip_v, flip_h);
            }
            else
            {
                for (int i = 0; i < count && written < total; i++, written++)
                {
                    if (_read_exact(fd, px, bpp) != 0) goto fail2;
                    _store_pixel(img, w, h, written, px, bpp, flip_v, flip_h);
                }
            }
        }
        else
        {
            if (_read_exact(fd, px, bpp) != 0) goto fail2;

            _store_pixel(img, w, h, written, px, bpp, flip_v, flip_h);
            written++;
        }
    }

    close(fd);
    return 0;

fail2:
    free(img->pixels);
    img->pixels = NULL;
fail:
    close(fd);
    return -1;
}

void tga_free(bmp_image_t *img)
{
    bmp_free(img);
}