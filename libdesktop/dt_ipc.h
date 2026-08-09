#pragma once

#include <sys/types.h>

#define DT_IPC_ROOT "/tmp/dt/"
#define DT_IPC_PATH_MAX 64

typedef enum
{
    DT_CHAN_CMD,
    DT_CHAN_DIRTY,
    DT_CHAN_WSIZE,
    DT_CHAN_INPUT,
    DT_CHAN_CURSOR,
} dt_chan_t;

void dt_ipc_itoa(int v, char *out);
void dt_ipc_path(dt_chan_t kind, pid_t pid, char out[DT_IPC_PATH_MAX]);
void dt_ipc_cmd_append(const char *line);

void dt_ipc_build_open_cmd(
    char *buf,
    int bufsz,
    pid_t pid,
    unsigned int style,
    int x,
    int y,
    int w,
    int h,
    const char *title
);

int dt_ipc_write(dt_chan_t kind, pid_t pid, const void *data, unsigned len);
int dt_ipc_read(dt_chan_t kind, pid_t pid, void *buf, unsigned max);
