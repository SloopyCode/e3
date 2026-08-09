#pragma once

#include "../win/win.h"
#include "../cmd/cmd.h"

void wm_clear_rect_dirty(int x, int y, int w, int h);
void wm_sync_home_to_current(void);
void wm_clear_cmd_rects(const cmd_result_t *cr);
void wm_clear_prev_drag_rect(void);

int wm_poll_client_damage(void);
