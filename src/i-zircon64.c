#include <dirent.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <threads.h>
#include <unistd.h>

#include <X11/keysym.h>
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>

#include "config.c"
#include "fonts.c"
#include "properties.c"
#include "ui.c"

uint32_t values[3];
char buffer[PIPE_BUF] = { 0 };
char* args[PIPE_BUF] = { 0 };

xcb_connection_t* connection;
xcb_screen_t* screen;
xcb_font_t font;
xcb_gcontext_t gc;
xcb_window_t window;
xcb_generic_event_t* event;
xcb_keysym_t keysym;

int key_position = 0;
int cursor_position = 0;

void
cursor()
{
  if (keysym == 65361)
    {
      if (cursor_position > 0)
        {
          cursor_position--;
        }
    }
  else if (keysym == 65363)
    {
      if (cursor_position < key_position)
        {
          cursor_position++;
        }
    }
  else if (keysym == 65360)
    {
      cursor_position = 0;
    }
  else if (keysym == 65367)
    {
      cursor_position = strlen(buffer);
    }

  int X_MOVE = 10 + cursor_position * 6;

  xcb_change_gc(connection, gc, XCB_GC_FOREGROUND, (uint32_t[]){ 0xf07178 });
  xcb_image_text_8(
    connection, strlen("|"), window, gc, X_MOVE, information.height / 10, "|");

  printf("Key position: %d, Cursor position: %d, Key: %u\n",
         key_position,
         cursor_position,
         keysym);
}

void
list_of_files()
{
  int files_count = 0;

  struct dirent* de;

  DIR* bindir = opendir("/usr/bin");
  if (!bindir)
    {
      perror("opendir(): ");
      exit(1);
    }

  int WIN_HORIZONTAL_FILES_ARRANGEMENT = 10, WIN_VERTICAL_FILES_ARRANGEMENT = 10;

  if (information.enable_horizontal_file_arrangement == true)
    {
      /* horizontal arrangement of files in the development */
      WIN_HORIZONTAL_FILES_ARRANGEMENT = information.width / 4;
    }
  else
    {
      WIN_VERTICAL_FILES_ARRANGEMENT = information.height / 4;
    }

  xcb_change_gc(connection, gc, XCB_GC_FOREGROUND, (uint32_t[]){ 0xf07178 });

  while ((de = readdir(bindir)) != NULL)
    {
      if (de->d_type != DT_DIR)
        {
          if (buffer[0] != '\0' && strncmp(buffer, de->d_name, strlen(buffer)) == 0)
            {
              xcb_image_text_8(connection,
                               strlen(de->d_name),
                               window,
                               gc,
                               (information.enable_horizontal_file_arrangement == true)
                                 ? (WIN_HORIZONTAL_FILES_ARRANGEMENT += 20)
                                 : 10,
                               (information.enable_horizontal_file_arrangement == false)
                                 ? (WIN_VERTICAL_FILES_ARRANGEMENT += 20)
                                 : 10,
                               de->d_name);
              files_count++;
              if (9 == (char)keysym && 1 == files_count)
                {
                  xcb_image_text_8(connection,
                                   strlen(de->d_name),
                                   window,
                                   gc,
                                   10,
                                   information.height / 10,
                                   de->d_name);
                  snprintf(buffer, sizeof(buffer), "%s", de->d_name);
                  cursor_position = strlen(buffer);
                  key_position = strlen(de->d_name);
                }
            }
          else if (0 == strlen(buffer))
            {
              xcb_image_text_8(connection,
                               strlen(de->d_name),
                               window,
                               gc,
                               (information.enable_horizontal_file_arrangement == true)
                                 ? (WIN_HORIZONTAL_FILES_ARRANGEMENT += 20)
                                 : 10,
                               (information.enable_horizontal_file_arrangement == false)
                                 ? (WIN_VERTICAL_FILES_ARRANGEMENT += 20)
                                 : 10,
                               de->d_name);
            }
        }
    }
  closedir(bindir);
}

void
init()
{
  connection = xcb_connect(NULL, NULL);
  if (connection == NULL)
    {
      printf("Error starting X11 server...");
      exit(1);
    }

  screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
  if (!screen)
    {
      printf("Error starting screen...");
      exit(1);
    }

  set_font();

  gc = xcb_generate_id(connection);
  window = screen->root;

  /* working with font */
  uint32_t mask = XCB_GC_BACKGROUND | XCB_GC_FONT;
  values[0] = information.font_bg_color; // font background color
  values[1] = font;

  xcb_create_gc(connection, gc, window, mask, values);

  window = xcb_generate_id(connection);

  /* working with window */
  mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  values[0] = information.bg_color; // background color
  values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS;

  xcb_create_gc(connection, gc, window, mask, values);

  xcb_create_window(connection,
                    screen->root_depth,
                    window,
                    screen->root,
                    (screen->width_in_pixels - information.width) / 2,
                    (screen->height_in_pixels - information.height) / 2,
                    information.width,
                    information.height,
                    1,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    screen->root_visual,
                    mask,
                    values);

  /* convert the window to the dock window type */
  make_dock();
  /* set the title of window */
  set_title();
  /* make windxow make_unresizable */
  make_unresizable();

  xcb_map_window(connection, window);
  xcb_flush(connection);

  xcb_get_input_focus(connection);

  xcb_set_input_focus(connection, XCB_INPUT_FOCUS_NONE, window, XCB_CURRENT_TIME);

  xcb_key_symbols_t* keysyms = xcb_key_symbols_alloc(connection);

  int arguments = 0;
  int status = 0;

  while ((event = xcb_wait_for_event(connection)))
    {
      switch (event->response_type & ~0x80)
        {
          case XCB_EXPOSE:
            xcb_change_gc(connection, gc, XCB_GC_FOREGROUND, "0x00000000");

            int width = information.width - 1;
            int heigth = information.height / 6;

            int16_t square_values[1][4] = { { 0, 0, width, heigth } };
            int16_t circle_values[1][6] = { {} };
            for (int i = 0; i < (int)sizeof(square_values) / sizeof(square_values[0]);
                 i++)
              {
                for (int j = 0;
                     j < (int)sizeof(square_values) / sizeof(square_values[0][0]);
                     j++)
                  {
                    draw_square(square_values[i][0],
                                square_values[i][1],
                                square_values[i][2],
                                square_values[i][3]);
                  }
              }

            for (int i = 0; i < (int)sizeof(circle_values) / sizeof(circle_values[0]);
                 i++)
              {
                for (int j = 0;
                     j < (int)sizeof(circle_values) / sizeof(circle_values[0][0]);
                     j++)
                  {
                    draw_circle(circle_values[i][0],
                                circle_values[i][1],
                                circle_values[i][2],
                                circle_values[i][3],
                                circle_values[i][4],
                                circle_values[i][5]);
                  }
              }

            list_of_files();
            cursor();

            xcb_flush(connection);

            break;
          case XCB_KEY_PRESS:
            xcb_key_press_event_t* key_event = (xcb_key_press_event_t*)event;
            keysym = xcb_key_symbols_get_keysym(keysyms, key_event->detail, 0);

            if (32 <= keysym && 126 >= keysym)
              {
                memmove(buffer + cursor_position + 1,
                        buffer + cursor_position,
                        strlen(buffer) - cursor_position);
                buffer[cursor_position] = keysym;
                cursor_position++;
                key_position++;
              }
            else
              {
                switch (keysym)
                  {
                    case XK_BackSpace:
                      if (cursor_position > 0)
                        {
                          memmove(&buffer[cursor_position - 1],
                                  &buffer[(cursor_position - 1) + 1],
                                  strlen(buffer));
                          key_position--;
                          cursor_position--;
                        }
                      break;

                    case XK_Return:
                      if (0 < key_position)
                        {
                          char* token = strtok(buffer, " ");
                          while (token != NULL)
                            {
                              printf("%s\n", token);
                              args[arguments++] = strdup(token);
                              token = strtok(NULL, " ");
                            }
                          args[arguments] = NULL;

                          pid_t pid = fork();
                          if (0 == pid)
                            {
                              execvp(args[0], args);
                              perror("execution failed");
                            }
                          else if (pid < 0)
                            {
                              perror("fork failed");
                            }

                          key_position = 0, arguments = 0, status = 1;
                          memset(args, 0, sizeof(args));
                        }
                      break;

                    case XK_Escape:
                      key_position = 0;
                      status = 1;
                      arguments = 0;
                      memset(args, 0, sizeof(args));
                  }

                if (key_event->state & XCB_MOD_MASK_CONTROL && keysym == XK_BackSpace)
                  {
                    memset(buffer, 0, sizeof(buffer));
                    memset(args, 0, sizeof(args));
                    cursor_position = 0;
                    key_position = 0;
                  }
              }

            xcb_clear_area(
              connection, 0, window, 0, 0, information.width, information.height);

            for (int i = 0; i < 2; i++)
              {
                for (int j = 0; j < 4; j++)
                  {
                    draw_square(square_values[i][0],
                                square_values[i][1],
                                square_values[i][2],
                                square_values[i][3]);
                  }
              }

            xcb_change_gc(
              connection, gc, XCB_GC_FOREGROUND, (uint32_t[]){ information.font_color });
            xcb_image_text_8(connection,
                             strlen(buffer),
                             window,
                             gc,
                             10,
                             information.height / 10,
                             buffer);

            list_of_files();
            cursor();

            xcb_flush(connection);

            if (1 == status)
              exit(1);
        }
    }

  xcb_key_symbols_free(keysyms);
  xcb_close_font(connection, font);
  xcb_disconnect(connection);
}

int
main(void)
{
  read_config_file();
  init();

  return EXIT_SUCCESS;
}
