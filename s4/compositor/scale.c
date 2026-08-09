/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: scale.c
 *
 */

#include "scale.h"

void scale_bilinear_region(
    const unsigned int *src,
    int src_w,
    int src_h,
    unsigned int *dst,
    int dst_w,
    int dst_h,
    unsigned int dst_stride,

    int dst_x0,
    int dst_y0,
    int dst_x1,
    int dst_y1
) {
    if (src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return;

    int hratio = (int)(((float)src_w / (float)dst_w) * 65536.0f);
    int vratio = (int)(((float)src_h / (float)dst_h) * 65536.0f);

    for (int y = dst_y0; y < dst_y1; y++)
    {
        int fy = y * vratio + (vratio / 2) - 32768;
        if (fy < 0) fy = 0;
        if (fy > (src_h - 1) * 65536) fy = (src_h - 1) * 65536;

        int ty = fy >> 16;
        int by = ty + 1;
        if (by >= src_h) by = src_h - 1;

        int vw = (fy >> 8) & 0xFF;
        int ovw = 256 - vw;

        for (int x = dst_x0; x < dst_x1; x++)
        {
            int fx = x * hratio + (hratio / 2) - 32768;
            if (fx < 0) fx = 0;
            if (fx > (src_w - 1) * 65536) fx = (src_w - 1) * 65536;

            int lx = fx >> 16;
            int rx = lx + 1;
            if (rx >= src_w) rx = src_w - 1;

            int hw = (fx >> 8) & 0xFF;
            int ohw = 256 - hw;

            int wtl = (ohw * ovw) >> 8;
            int wtr = (hw * ovw) >> 8;
            int wbl = (ohw * vw) >> 8;
            int wbr = (hw * vw) >> 8;

            unsigned int ptl = src[ty * src_w + lx];
            unsigned int ptr = src[ty * src_w + rx];
            unsigned int pbl = src[by * src_w + lx];
            unsigned int pbr = src[by * src_w + rx];

            unsigned int a =
                ((ptl >> 24 & 0xFF) * wtl +
                (ptr >> 24 & 0xFF) * wtr +
                (pbl >> 24 & 0xFF) * wbl +
                (pbr >> 24 & 0xFF) * wbr)
                >> 8
            ;
            unsigned int r =
                ((ptl >> 16 & 0xFF) * wtl +
                (ptr >> 16 & 0xFF) * wtr +
                (pbl >> 16 & 0xFF) * wbl +
                (pbr >> 16 & 0xFF) * wbr)
                >> 8
            ;
            unsigned int g =
                ((ptl >> 8  & 0xFF) * wtl +
                (ptr >> 8  & 0xFF) * wtr +
                (pbl >> 8  & 0xFF) * wbl +
                (pbr >> 8  & 0xFF) * wbr)
                >> 8
            ;
            unsigned int b =
                ((ptl & 0xFF) * wtl +
                (ptr & 0xFF) * wtr +
                (pbl & 0xFF) * wbl +
                (pbr & 0xFF) * wbr)
                >> 8
            ;

            dst[y * dst_stride + x] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }
}
