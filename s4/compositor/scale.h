/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: scale.h
 *
 */

#pragma once

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
);
