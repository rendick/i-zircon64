#include <stdlib.h>
#include <string.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xproto.h>

#include "config.c"

xcb_connection_t *connection;
xcb_gcontext_t gc;
xcb_window_t window;

void make_dock() {
  xcb_atom_t window_type_atom, window_type_dock_atom;
  xcb_intern_atom_cookie_t atom_cookie = xcb_intern_atom(
      connection, 0, strlen("_NET_WM_WINDOW_TYPE"), "_NET_WM_WINDOW_TYPE");
  xcb_intern_atom_reply_t *atom_reply =
      xcb_intern_atom_reply(connection, atom_cookie, NULL);
  if (atom_reply) {
    window_type_atom = atom_reply->atom;
    free(atom_reply);

    if (1) {
      atom_cookie =
          xcb_intern_atom(connection, 0, strlen("_NET_WM_WINDOW_TYPE_DOCK"),
                          "_NET_WM_WINDOW_TYPE_DOCK");
    } else {
      atom_cookie =
          xcb_intern_atom(connection, 0, strlen("_NET_WM_WINDOW_TYPE_DIALOG"),
                          "_NET_WM_WINDOW_TYPE_DIALOG");
    }
    atom_reply = xcb_intern_atom_reply(connection, atom_cookie, NULL);
    if (atom_reply) {
      window_type_dock_atom = atom_reply->atom;
      free(atom_reply);
      xcb_change_property_checked(connection, XCB_PROP_MODE_REPLACE, window,
                                  window_type_atom, XCB_ATOM_ATOM, 32, 1,
                                  &window_type_dock_atom);
    }
  }

  atom_cookie = xcb_intern_atom(connection, 0, strlen("_NET_WM_DESKTOP"),
                                "_NET_WM_DESKTOP");
  atom_reply = xcb_intern_atom_reply(connection, atom_cookie, NULL);

  xcb_change_property_checked(connection, XCB_PROP_MODE_REPLACE, window,
                              atom_reply->atom, XCB_ATOM_ATOM, 32, 1,
                              (const uint32_t[]){0xFFFFFFFF});
}

void set_title() {
  xcb_icccm_set_wm_name(connection, window, XCB_ATOM_STRING, 8, strlen("I-ZIRCON64"),
                        "I-ZIRCON64");
}

void make_unresizable() {
  xcb_size_hints_t window_size_hints;
  xcb_icccm_size_hints_set_min_size(&window_size_hints, WIN_WIDTH, WIN_HEIGHT);
  xcb_icccm_size_hints_set_max_size(&window_size_hints, WIN_WIDTH, WIN_HEIGHT);
  xcb_icccm_size_hints_set_win_gravity(&window_size_hints, XCB_GRAVITY_CENTER);
  xcb_icccm_set_wm_size_hints(connection, window, XCB_ATOM_WM_NORMAL_HINTS,
                              &window_size_hints);
}