#include <stdint.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

xcb_connection_t *connection;
xcb_gcontext_t gc;
xcb_window_t window;

void draw_square(int x, int y, int w, int h) {
  xcb_change_gc(connection, gc, XCB_GC_FOREGROUND, (uint32_t[]){0x434145});
  xcb_poly_fill_rectangle(connection, window, gc, 1,
                          (xcb_rectangle_t[]){{x, y, w, h}});
}

void draw_circle(int x, int y, int h, int w, int angle1, int angle2) {
  xcb_change_gc(connection, gc, XCB_GC_FOREGROUND, (uint32_t[]){0xffffff});
  xcb_poly_fill_arc(connection, window, gc, 1,
                    (xcb_arc_t[]){{x, y, h, w, angle1, angle2}});
}
