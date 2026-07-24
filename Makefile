#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (c) 2026 doccrLabs
#
# PROJECT: e3
# FILE: Makefile
# CREATED BY: emex
# MODIFIED BY: --
#
#

CC     := x86_64-elf-gcc
LD     := x86_64-elf-ld
LIBC   ?= include/libc

OS_PATH     ?= ~/doccrLabs/doccrOS/
ROOTFS_PATH ?= $(OS_PATH)dsk/rd/system/desktop/
OS_LIBS     ?= $(OS_PATH)/user/libs/
LIBDESKTOP  := libdesktop/

CFLAGS := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector     \
          -fno-PIE -fno-pic -m64 -march=x86-64 -mno-sse -mno-sse2        \
          -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11 -D__doccr__ -g \
          -I$(LIBC)/include                                              \
          -I$(LIBDESKTOP)

LDFLAGS := -nostdlib -static -no-pie -T user.ld

LIBC_DIR := include/libc

OBJS := build/desktop.o \
		build/desktop_input_dispatch.o \
        build/compositor/comp.o \
        build/cmd/cmd.o \
        build/bg/bmp/bmp.o \
        build/bg/bg.o \
        build/win/win.o \
        build/cursor/cursor.o \
        build/render/render.o \
        build/input/input.o \
        build/fonts/fonts.o \
        build/taskbar/taskbar.o \
        build/taskbar/entries.o \
        build/taskbar/startmenu.o

all: clean $(LIBC)/build/crt0.o dirs build/desktop.elf run

fetchDeps:
	mkdir -p include
	if [ ! -d "$(LIBC_DIR)" ]; then \
		git clone --depth=1 https://github.com/doccrLabs/doccrOS.git include/doccrOS; \
		mv include/doccrOS/user/libc $(LIBC_DIR); \
		rm -rf include/doccrOS; \
	fi

dirs:
	mkdir -p libdesktop/build

	mkdir -p build
	mkdir -p build/compositor
	mkdir -p build/bg
	mkdir -p build/bg/bmp
	mkdir -p build/win
	mkdir -p build/cursor
	mkdir -p build/render
	mkdir -p build/input
	mkdir -p build/cmd
	mkdir -p build/fonts
	mkdir -p build/taskbar
run:
	@echo "running OS..."
	cd $(OS_PATH) && make run

build/desktop.elf: dirs $(OBJS) $(LIBC)/build/crt0.o $(LIBC)/build/libc.a $(LIBDESKTOP)/build/libdesktop.a
	@echo "Building e3 now..."
	$(LD) $(LDFLAGS) $(LIBC)/build/crt0.o $(OBJS) $(LIBDESKTOP)/build/libdesktop.a $(LIBC)/build/libc.a -o $@
	@echo "e3 was succesfully built!"
	@rm -f  $(ROOTFS_PATH)/desktop.elf
	@cp build/desktop.elf $(ROOTFS_PATH)desktop.elf
	@cp libdesktop/build/libdesktop.a $(OS_LIBS)libdesktop.a
	@cp libdesktop/libdesktop.h $(OS_LIBS)libdesktop.h


build/desktop.o:                    src/desktop.c                   ; $(CC) $(CFLAGS) -c $< -o $@
build/compositor/comp.o:            src/compositor/comp.c           ; $(CC) $(CFLAGS) -c $< -o $@
build/bg/bmp/bmp.o:                 src/bg/bmp/bmp.c                ; $(CC) $(CFLAGS) -c $< -o $@
build/bg/bg.o:                      src/bg/bg.c                     ; $(CC) $(CFLAGS) -c $< -o $@
build/win/win.o:                    src/win/win.c                   ; $(CC) $(CFLAGS) -c $< -o $@
build/desktop_input_dispatch.o:     src/desktop_input_dispatch.c    ; $(CC) $(CFLAGS) -c $< -o $@
build/cursor/cursor.o:              src/cursor/cursor.c             ; $(CC) $(CFLAGS) -c $< -o $@
build/render/render.o:              src/render/render.c             ; $(CC) $(CFLAGS) -c $< -o $@
build/input/input.o:                src/input/input.c               ; $(CC) $(CFLAGS) -c $< -o $@
build/fonts/fonts.o:                src/fonts/fonts.c               ; $(CC) $(CFLAGS) -c $< -o $@
build/taskbar/taskbar.o:            src/taskbar/taskbar.c           ; $(CC) $(CFLAGS) -c $< -o $@
build/taskbar/entries.o:            src/taskbar/entries.c           ; $(CC) $(CFLAGS) -c $< -o $@
build/taskbar/startmenu.o:          src/taskbar/startmenu.c         ; $(CC) $(CFLAGS) -c $< -o $@
build/cmd/cmd.o:                    src/cmd/cmd.c                   ; $(CC) $(CFLAGS) -c $< -o $@

$(LIBC)/build/crt0.o $(LIBC)/build/libc.a:
	$(MAKE) -C $(LIBC)

$(LIBDESKTOP)/build/libdesktop.a:
	$(MAKE) -C $(LIBDESKTOP)

clean:
	rm -f build/*.o build/compositor/*.o build/bg/*.o build/bg/bmp/*.o build/win/*.o build/cursor/*.o build/render/*.o build/input/*.o build/cmd/*.o build/fonts/* build/taskbar/*.o build/desktop.elf

.PHONY: all clean install run
