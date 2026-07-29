/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: shm_host.c
 *
 */

#include "shm_host.h"
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

static int s_fd = -1;

void shm_host_init(void)
{
    s_fd = open(SHM_DEV, O_RDWR);
    if (s_fd < 0)
    {
        printf("[SHM] warning: could not open " SHM_DEV "\n");
    }
}

unsigned int *shm_host_map(unsigned long long id)
{
    if (s_fd < 0) return NULL;

    shm_ioctl_args_t args =
    {
	    .id = id,
	    .size = 0,
	    .vaddr = 0
    };
    if (ioctl(s_fd, SHM_IOCTL_MAP, &args) < 0) return NULL;

    return (unsigned int *)(unsigned long)args.vaddr;
}

void shm_host_unmap(unsigned long long id, unsigned int *ptr)
{
    if (s_fd < 0 || !ptr) return;

    shm_ioctl_args_t args =
    {
	    .id = id,
	    .size = 0,
	    .vaddr = (uint64_t)(unsigned long)ptr
    };
    ioctl(s_fd, SHM_IOCTL_UNMAP, &args);
}