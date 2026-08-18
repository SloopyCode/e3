#include "dt_ipc.h"
#include <fcntl.h>
#include <unistd.h>

#define DT_IPC_CMD_NAME "s4.desktop.command"
#define DT_IPC_EVENT_PREFIX "s4.desktop.event."
#define IPC_NONBLOCK 1

static int kernel_ipc = -1;
static int command_fd = -1;
static int event_fd = -1;
static pid_t event_pid = 0;

static int ipc_available(void)
{
    /*if (kernel_ipc >= 0) return kernel_ipc;
    command_fd = (int)ipc_open(DT_IPC_CMD_NAME);
    kernel_ipc = command_fd >= 0;
    return kernel_ipc;*/
    return 0;
}

static void event_name(pid_t pid, char out[DT_IPC_PATH_MAX])
{
    int i = 0;
    int j = 0;
    const char *prefix = DT_IPC_EVENT_PREFIX;
    char number[12];

    while (prefix[i]) { out[i] = prefix[i]; i++; }
    dt_ipc_itoa((int)pid, number);
    while (number[j]) out[i++] = number[j++];

    out[i] = '\0';
}

static int event_channel(pid_t pid, int create)
{
    if (event_fd >= 0 && event_pid == pid) return event_fd;

    char name[DT_IPC_PATH_MAX];
    event_name(pid, name);
    event_fd = create ? (int)ipc_create(name) : (int)ipc_open(name);

    if (event_fd >= 0) event_pid = pid;

    return event_fd;
}

int dt_ipc_desktop_init(void)
{
    command_fd = (int)ipc_create(DT_IPC_CMD_NAME);
    kernel_ipc = command_fd >= 0;
    return kernel_ipc ? (int)ipc_eventfd(command_fd) : -1;
}

int dt_ipc_uses_kernel(void)
{
    return ipc_available();
}

void dt_ipc_itoa(int v, char *out)
{
    char tmp[16];
    int i = 0;
    int j = 0;
    int neg = (v < 0);

    if (v == 0)
    {
        out[0] = '0';
        out[1] = '\0';
        return;
    }

    if (neg) v = -v;

    while (v)
    {
        tmp[i++] = '0' + v % 10;
        v /= 10;
    }

    if (neg) tmp[i++] = '-';

    while (i > 0) out[j++] = tmp[--i];

    out[j] = '\0';
}

static const char *chan_prefix(dt_chan_t kind)
{
    switch (kind)
    {
        case DT_CHAN_CMD:    return DT_IPC_ROOT "cmd";
        case DT_CHAN_DIRTY:  return DT_IPC_ROOT "dirty_";
        case DT_CHAN_WSIZE:  return DT_IPC_ROOT "wsize_";
        case DT_CHAN_INPUT:  return DT_IPC_ROOT "input_";
        case DT_CHAN_CURSOR: return DT_IPC_ROOT "cursor";
        default: return DT_IPC_ROOT "?";
    }
}

static int chan_is_keyed(dt_chan_t kind)
{
    return
        kind == DT_CHAN_DIRTY ||
        kind == DT_CHAN_WSIZE ||
        kind == DT_CHAN_INPUT
    ;
}

void dt_ipc_path(dt_chan_t kind, pid_t pid, char out[DT_IPC_PATH_MAX])
{
    const char *pfx = chan_prefix(kind);
    int i = 0;

    while (*pfx) out[i++] = *pfx++;

    if (chan_is_keyed(kind))
    {
        char ps[12];
        int j = 0;

        dt_ipc_itoa((int)pid, ps);
        while (ps[j]) out[i++] = ps[j++];
    }

    out[i] = '\0';
}

int dt_ipc_write(dt_chan_t kind, pid_t pid, const void *data, unsigned len)
{
    if (kind == DT_CHAN_INPUT && ipc_available())
    {
        int fd = event_channel(pid, 0);
        if (fd >= 0) return (int)ipc_send(fd, data, len);
    }

    char path[DT_IPC_PATH_MAX];
    dt_ipc_path(kind, pid, path);

    int fd = open(path, O_WRONLY | O_CREAT);
    if (fd < 0) return -1;
    if (ftruncate(fd, 0) < 0) { close(fd); return -1; }
    int w = (int)write(fd, data, len);
    close(fd);
    return w;
}

int dt_ipc_read(dt_chan_t kind, pid_t pid, void *buf, unsigned max)
{
    if (kind == DT_CHAN_INPUT && ipc_available())
    {
        int fd = event_channel(pid, 1);
        if (fd >= 0) return (int)ipc_recv(fd, buf, max, IPC_NONBLOCK);
    }

    char path[DT_IPC_PATH_MAX];
    dt_ipc_path(kind, pid, path);

    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;

    int r = (int)read(fd, buf, max);
    close(fd);
    return r;
}

void dt_ipc_cmd_append(const char *line)
{
    if (ipc_available())
    {
        int len = 0;
        while (line[len]) len++;
        if (ipc_send(command_fd, line, (unsigned)len) >= 0) return;
    }
    static char existing[4096];
    int elen = 0;

    char path[DT_IPC_PATH_MAX];
    dt_ipc_path(DT_CHAN_CMD, 0, path);

    int fd = open(path, O_RDONLY);
    if (fd >= 0)
    {
        int r = (int)read(fd, existing, sizeof(existing) - 1);
        close(fd);
        if (r > 0) elen = r;
    }

    fd = open(path, O_WRONLY | O_CREAT);
    if (fd < 0) return;

    if (ftruncate(fd, 0) < 0) { close(fd); return; }

    if (elen > 0) write(fd, existing, (unsigned)elen);

    {
        int llen = 0;
        while (line[llen]) llen++;
        write(fd, line, (unsigned)llen);
    }

    close(fd);
}

void dt_ipc_cmd_clear(void)
{
    if (ipc_available()) return;

    char path[DT_IPC_PATH_MAX];
    dt_ipc_path(DT_CHAN_CMD, 0, path);

    int fd = open(path, O_WRONLY | O_CREAT);
    if (fd < 0) return;

    ftruncate(fd, 0);

    close(fd);
}

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
) {
    (void)bufsz;
    int p = 0;
    char num[12];
    int k;

    buf[p++] = 'O';
    buf[p++] = ' ';

    dt_ipc_itoa((int)pid, num);
    for (k = 0; num[k]; k++) buf[p++] = num[k]; buf[p++] = ' ';

    dt_ipc_itoa((int)style, num);
    for (k = 0; num[k]; k++) buf[p++] = num[k]; buf[p++] = ' ';

    dt_ipc_itoa(x, num);
    for (k = 0; num[k]; k++) buf[p++] = num[k]; buf[p++] = ' ';

    dt_ipc_itoa(y, num);
    for (k = 0; num[k]; k++) buf[p++] = num[k]; buf[p++] = ' ';

    dt_ipc_itoa(w, num);
    for (k = 0; num[k]; k++) buf[p++] = num[k]; buf[p++] = ' ';

    dt_ipc_itoa(h, num);
    for (k = 0; num[k]; k++) buf[p++] = num[k]; buf[p++] = ' ';

    if (title)
    {
        const char *t = title;
        while (*t) buf[p++] = *t++;
    }

    buf[p++] = '\n';
    buf[p] = '\0';
}
