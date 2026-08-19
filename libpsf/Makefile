#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (c) 2026 sulfurLabs
#
# PROJECT: s4
# FILE: libpsf/Makefile
#

CC  := x86_64-elf-gcc
AR  := x86_64-elf-ar
LIBC := ../include/libc

CFLAGS := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector  \
          -fno-PIE -fno-pic -m64 -march=x86-64 -mno-red-zone          \
          -Wall -Wextra -std=gnu11 -D__sulfur__                       \
          -I$(LIBC)/include                                           \
          -I.

SRCS := psf.c
OBJS := $(SRCS:%.c=build/%.o)

all: build/psf.a

build/%.o: %.c psf.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build/psf.a: $(OBJS) | build
	$(AR) rcs $@ $(OBJS)

build:
	mkdir -p build

clean:
	rm -rf build

.PHONY: all clean
