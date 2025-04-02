
#include <string.h>
#include <xcb/xcb.h>

#include "config.c"

xcb_connection_t *connection;
xcb_font_t font;

xcb_gcontext_t gc;
xcb_window_t window;

void set_font() {
  font = xcb_generate_id(connection);
  xcb_open_font(connection, font, strlen(FONT), FONT);
}
