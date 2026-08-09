#include "wm.h"
#include "../render/render_target.h"
#include "../bg/bg.h"
#include "../ipc/ipc.h"

extern drag_info_t g_input_drag_prev;

void wm_clear_rect_dirty(int x, int y, int w, int h)
{
    bg_draw_rect(x, y, w, h);
    rt_damage_mark(x, y, w, h);
}

static void current_content(dt_win_t *w, int *cx, int *cy, int *cw, int *ch)
{
    unsigned int s = w->style;

    if (s & DT_POPUP)
    {
        *cx = w->x + 1;
        *cy = w->y + 1;
        *cw = w->w - 2;
        *ch = w->h - 2;
    }
    else if (s & DT_NOTITLE)
    {
        *cx = w->x + DT_BORDER;
        *cy = w->y + DT_BORDER;
        *cw = w->w - DT_BORDER * 2;
        *ch = w->h - DT_BORDER * 2;
    }
    else
    {
        *cx = w->x + DT_BORDER;
        *cy = w->y + DT_TITLE_H + 1;
        *cw = w->w - DT_BORDER * 2;
        *ch = w->h - DT_TITLE_H - 1 - DT_BORDER;
    }
}

static void old_full_rect(
    dt_win_t *w,
    int hcx,
    int hcy,
    int hcw,
    int hch,

    int *ox,
    int *oy,
    int *ow,
    int *oh
) {
    unsigned int s = w->style;

    if (s & DT_POPUP)
    {
        *ox = hcx - 1;
        *oy = hcy - 1;
        *ow = hcw + 2;
        *oh = hch + 2;
    }
    else if (s & DT_NOTITLE)
    {
        *ox = hcx - DT_BORDER;
        *oy = hcy - DT_BORDER;
        *ow = hcw + DT_BORDER * 2;
        *oh = hch + DT_BORDER * 2;
    }
    else
    {
        *ox = hcx - DT_BORDER;
        *oy = hcy - DT_TITLE_H - 1;
        *ow = hcw + DT_BORDER * 2;
        *oh = hch + DT_TITLE_H + 1 + DT_BORDER;
    }
}

void wm_sync_home_to_current(void)
{
    for (int i = 0; i < DT_WIN_MAX; i++)
    {
        dt_win_t *w = win_get(i);
        if (!w) continue;

        int cx;
        int cy;
        int cw;
        int ch;
        int ox;
        int oy;
        int ow;
        int oh;
        int nox;
        int noy;
        int now_;
        int noh;

        current_content(w, &cx, &cy, &cw, &ch);

        if (cx == w->home_cx && cy == w->home_cy && cw == w->home_cw && ch == w->home_ch) continue;

        old_full_rect(w, w->home_cx, w->home_cy, w->home_cw, w->home_ch, &ox, &oy, &ow, &oh);
        old_full_rect(w, cx, cy, cw, ch, &nox, &noy, &now_, &noh);

        int ix0 = ox > nox ? ox : nox;
        int iy0 = oy > noy ? oy : noy;
        int ix1 = (ox + ow) < (nox + now_) ? (ox + ow) : (nox + now_);
        int iy1 = (oy + oh) < (noy + noh) ? (oy + oh) : (noy + noh);

        if (ix0 >= ix1 || iy0 >= iy1)
        {
            wm_clear_rect_dirty(ox, oy, ow, oh);
        }
        else
        {
            if (oy < iy0) wm_clear_rect_dirty(ox, oy, ow, iy0 - oy);
            if (oy + oh > iy1) wm_clear_rect_dirty(ox, iy1, ow, (oy + oh) - iy1);

            if (iy0 < iy1)
            {
                if (ox < ix0) wm_clear_rect_dirty(ox, iy0, ix0 - ox, iy1 - iy0);
                if (ox + ow > ix1) wm_clear_rect_dirty(ix1, iy0, (ox + ow) - ix1, iy1 - iy0);
            }
        }

        rt_damage_mark(nox, noy, now_, noh);

        w->home_cx = cx;
        w->home_cy = cy;
        w->home_cw = cw;
        w->home_ch = ch;
    }
}

void wm_clear_cmd_rects(const cmd_result_t *cr)
{
    for (int i = 0; i < cr->count; i++)
    {
        const drag_info_t *d = &cr->rects[i];
        if (!d->valid) continue;

        wm_clear_rect_dirty(d->wx, d->wy, d->ww, d->wh);

        int idx = win_find_pid(d->pid);
        dt_win_t *wn = win_get(idx);
        if (wn) rt_damage_mark(wn->x, wn->y, wn->w, wn->h);
    }
}

void wm_clear_prev_drag_rect(void)
{
    if (!g_input_drag_prev.valid) return;

    wm_clear_rect_dirty(
        g_input_drag_prev.wx,
        g_input_drag_prev.wy,
        g_input_drag_prev.ww,
        g_input_drag_prev.wh
    );

    g_input_drag_prev.valid = 0;
}

int wm_poll_client_damage(void)
{
    int any = 0;

    for (int i = 0; i < DT_WIN_MAX; i++)
    {
        dt_win_t *wn = win_get(i);
        if (!wn || !wn->buf) continue;
        if (!ipc_window_is_dirty(wn->pid)) continue;

        rt_damage_mark(wn->x, wn->y, wn->w, wn->h);
        any = 1;
    }

    return any;
}
