/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: shm_host.h
 *
 */
#pragma once

void shm_host_init(void);
unsigned int *shm_host_map(unsigned long long id);
void shm_host_unmap(unsigned long long id, unsigned int *ptr);