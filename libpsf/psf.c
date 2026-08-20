/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: psf.c
 */

#include "psf.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// the terminus PSF (v14n) is psf2 so we just need to support that

#define PSF2_MAGIC0 0x72
#define PSF2_MAGIC1 0xb5
#define PSF2_MAGIC2 0x4a
#define PSF2_MAGIC3 0x86

typedef struct __attribute__((packed))
{
    uint8_t magic[4]; //0x72 0xb52 0x4a 0x86
    uint32_t version;
    #define PSF2_MAXVERSION 0
    uint32_t headersize;
    uint32_t flags;
    #define PSF2_HAS_UNICODE_TABLE 0x01
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
} psf2_header_t;

// we have no utf8 std lib yet
static uint32_t utf8_decode(const uint8_t *buf, uint32_t len, uint32_t *pos)
{
    uint8_t b0 = buf[*pos];

    if (b0 < 0x80)
    {
        (*pos)++;

        return b0;
    }

    if ((b0 & 0xE0) == 0xC0 && *pos + 1 < len)
    {
        uint32_t cp = ((uint32_t)(b0 & 0x1F) << 6) | (buf[*pos + 1] & 0x3F);

        *pos += 2;

        return cp;
    }

    if ((b0 & 0xF0) == 0xE0 && *pos + 2 < len)
    {
        uint32_t cp =
            ((uint32_t)(b0 & 0x0F) << 12)           | // always + 6
            ((uint32_t)(buf[*pos + 1] & 0x3F) << 6) | // always + 6
            ( buf[*pos + 2] & 0x3F)
        ;

        *pos += 3;

        return cp;
    }

    if ((b0 & 0xF8) == 0xF0 && *pos + 3 < len)
    {
        uint32_t cp =
            ((uint32_t)(b0 & 0x07) << 18)            | // always + 6
            ((uint32_t)(buf[*pos + 1] & 0x3F) << 12) | // always + 6
            ((uint32_t)(buf[*pos + 2] & 0x3F) << 6) | // always + 6
            ( buf[*pos + 3] & 0x3F)
        ;

        *pos += 4;

        return cp;
    }

    (*pos)++;
    return 0xFFFD;
}

static int add_unicode_entry(
    psf_font_t *font,
    uint32_t *cap,
    uint32_t codepoint,
    uint32_t glyph_index
) {
    if (font->unicode_entry_count >= *cap)
    {
        uint32_t new_cap = (*cap == 0) ? 64 : (*cap * 2);
        psf_unicode_entry_t *n = (psf_unicode_entry_t *)realloc(
            font->unicode_entries,
            (size_t)new_cap * sizeof(psf_unicode_entry_t)
        );

        if (!n) return -1;
        font->unicode_entries = n;
        *cap = new_cap;
    }

    font->unicode_entries[font->unicode_entry_count].codepoint = codepoint;
    font->unicode_entries[font->unicode_entry_count].glyph = glyph_index;

    font->unicode_entry_count++;

    return 0;
}

static void parse_unicode_table(psf_font_t *font, const uint8_t *table, uint32_t size)
{
    uint32_t pos = 0;
    uint32_t glyph = 0;
    uint32_t cap = 0;

    font->unicode_entries = NULL;
    font->unicode_entry_count = 0;

    while (pos < size && glyph < font->glyph_count)
    {
        uint8_t b = table[pos];

        if (b == 0xFF)
        {
            pos++;
            glyph++;
            continue;
        }
        if (b == 0xFE)
        {
            pos++;
            continue;
        }

        uint32_t cp = utf8_decode(table, size, &pos);

        if (add_unicode_entry(font, &cap, cp, glyph) != 0)
        {
            break;
        }
    }
}

int psf_load(const char *path, psf_font_t *font)
{
    //printf("\n\ntest0\n\n");
    if (!path)
    {
        //printf("test1");

        return -1;
    }

    if (!font)
    {
        //printf("test2");

        return -1;
    }

    psf2_header_t hdr;

    font->version = 0;
    font->glyph_count = 0;
    font->bytes_per_row = 0;
    font->bytes_per_glyph = 0;
    font->height = 0;
    font->width = 0;
    font->glyphs = NULL;
    font->unicode_entries = NULL;
    font->unicode_entry_count = 0;

    int fd = (int)open(path, O_RDONLY);
    if (fd < 0) return -1;

    if (read(fd, &hdr, sizeof(hdr)) != (long)sizeof(hdr))
    {
        //printf("test3");
        close(fd);
        return -1;
    }

    uint32_t glyph_data_size = hdr.numglyph * hdr.bytesperglyph;
    uint8_t *glyphs = (uint8_t *)malloc(glyph_data_size);

    if (
        hdr.magic[0] != PSF2_MAGIC0  ||
        hdr.magic[1] != PSF2_MAGIC1  ||
        hdr.magic[2] != PSF2_MAGIC2  ||
        hdr.magic[3] != PSF2_MAGIC3
    ) {
        // its either not even a psf file or its psf
        // type 1 which we cannot parse (cuz the desktop deosnt need it)

        //printf("test4");
        close(fd);
        return -1;
    }

    if (
        hdr.width == 0  ||
        hdr.height == 0 ||
        hdr.numglyph == 0
    ) {
        //printf("test4");
        close(fd);
        return -1;
    }

    if (lseek(fd, (long)hdr.headersize, SEEK_SET) < 0)
    {
        //printf("test5");
        close(fd);
        return -1;
    }

    if (!glyphs)
    {
        //printf("test6");
        close(fd);
        return -1;
    }

    if (read(fd, glyphs, glyph_data_size) != (long)glyph_data_size)
    {
        //printf("test7");
        free(glyphs);
        //printf("test8");
        close(fd);

        return -1;
    }

    font->version = hdr.version;
    font->glyph_count = hdr.numglyph;
    font->bytes_per_glyph = hdr.bytesperglyph;
    font->bytes_per_row = (hdr.width + 7) / 8;
    font->height = hdr.height;
    font->width = hdr.width;
    font->glyphs = glyphs;

    if (hdr.flags & PSF2_HAS_UNICODE_TABLE)
    {
        long cur = lseek(fd, 0, SEEK_CUR);
        long end = lseek(fd, 0, SEEK_END);

        if (cur >= 0 && end > cur)
        {
            long table_size = end - cur;
            lseek(fd, cur, SEEK_SET);

            uint8_t *table = (uint8_t *) malloc((size_t)table_size);
            if (table)
            {
                if (read(fd, table, (size_t)table_size) == table_size)
                {
                    parse_unicode_table(font, table, (size_t)table_size);
                }
                free(table);
            }
        }
    }

    //printf("test9");
    close(fd);
    return 0;
}

void psf_free(psf_font_t *font)
{
    if (font->glyphs) free(font->glyphs);
    if (font->unicode_entries) free(font->unicode_entries);

    font->glyph_count = 0;
    font->unicode_entries = NULL;
    font->glyphs = NULL;
    font->unicode_entry_count = 0;
}

int psf_glyph_index(const psf_font_t *font, uint32_t codepoint)
{
    if (font->unicode_entries)
    {
        for(uint32_t i = 0; i < font->unicode_entry_count; i++)
        {
            if (font->unicode_entries[i].codepoint == codepoint)
            {
                return (int)font->unicode_entries[i].glyph;
            }
        }

        return -1;
    }

    // if no unicode table the font is indexed per codepage
    // (does work on plain ascii btw)
    if (codepoint < font->glyph_count) return (int)codepoint;

    return -1;
}

int psf_get_pixel(const psf_font_t *font, uint32_t glyph_index, int x, int y)
{
    if (!font || !font->glyphs) return 0;
    if (glyph_index >= font->glyph_count) return 0;
    if (x < 0 || x >= (int)font->width || y < 0 || y >= (int)font->height) return 0;

    const uint8_t *g = font->glyphs + (size_t)glyph_index * font->bytes_per_glyph;
    const uint8_t *row = g + (size_t)y * font->bytes_per_row;
    uint8_t byte = row[x / 8];

    return (byte >> ( 7 - (x % 8))) & 1;
}

uint32_t psf_glyph_row_bits(const psf_font_t *font, uint32_t glyph_index, int row)
{
    if (!font) return 0;
    if (glyph_index >= font->glyph_count) return 0;
    if (row < 0 || row >= (int)font->height) return 0;

    int w = (int)font->width;
    if (w > 32) w = 32;

    uint32_t bits = 0;

    for (int x = 0; x < w; x++)
    {
        if (psf_get_pixel(font, glyph_index, x, row)) bits |= 1u << x;
    }

    return bits;
}

int psf_width(const psf_font_t *font)
{
    return font ? (int)font->width : 0;
}
int psf_height(const psf_font_t *font)
{
    return font ? (int)font->height : 0;
}
