#pragma once

#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define MAX_BYTES 128

char c_buffer[PIPE_BUF];
char* cutted_buffer[PIPE_BUF] = { 0 };
char config_dir_path[256];

typedef struct
{
  char font[MAX_BYTES];
  uint32_t bg_color;
  uint32_t font_color;
  uint32_t font_bg_color;
  int16_t width;
  int16_t height;
  int16_t enable_horizontal_file_arrangement;
} Information;

Information information;

int
get_pwd_to_plugins_dir()
{
  char* username = getenv("USER");
  if (NULL == username)
    return EXIT_FAILURE;
  snprintf(config_dir_path, 256, "/home/%s/.config/i-zircon64/init.el", username);
  return EXIT_SUCCESS;
}

int
read_config_file()
{
  get_pwd_to_plugins_dir();
  
  FILE* rcf = fopen(config_dir_path, "r");
  if (NULL == rcf)
    {
      printf("Error reading configuration file. Check the existence of files.\n");
      exit(1);
    }
  
  int lines = 0;
  int arguments = 0;
  while (fgets(c_buffer, sizeof(c_buffer), rcf))
    {
      lines++;
      char* token = strtok(c_buffer, " ");
      while (token != NULL)
        {
          arguments++;
          if (strcmp(c_buffer, "\n") != 0)
            {
              if (arguments == 1)
                {
                  if (strcmp(token, "(defun") != 0) {
                      printf("config.el: error on the line %d\n", lines);
                      exit(1);
                    }
                }
            }

          cutted_buffer[arguments] = token;
          token = strtok(NULL, " ");
        }

      cutted_buffer[3][strlen(cutted_buffer[3]) - 1] = '\0';
      cutted_buffer[3][strlen(cutted_buffer[3]) - 1] = '\0';

      for (int j = 1; j <= arguments; j++)
        {
          if (strcmp(cutted_buffer[j], "font") == 0)
            {
              strcpy(information.font, cutted_buffer[j + 1]);
            }
          else if (strcmp(cutted_buffer[j], "bg-color") == 0)
            {
              information.bg_color = strtol(cutted_buffer[j + 1], NULL, 0);
            }
          else if (strcmp(cutted_buffer[j], "font-color") == 0)
            {
              information.font_color = strtol(cutted_buffer[j + 1], NULL, 0);
            }
          else if (strcmp(cutted_buffer[j], "font-bg-color") == 0)
            {
              information.font_bg_color = strtol(cutted_buffer[j + 1], NULL, 0);
            }
          else if (strcmp(cutted_buffer[j], "width") == 0)
            {
              information.width = atoi(cutted_buffer[j + 1]);
            }
          else if (strcmp(cutted_buffer[j], "height") == 0)
            {
              information.height = atoi(cutted_buffer[j + 1]);
            }
          else if (strcmp(cutted_buffer[j], "enable-horizontal-file-arrangement") == 0)
            {
              information.enable_horizontal_file_arrangement = atoi(cutted_buffer[j + 1]);
            }
        }

      arguments = 0;
    }

  fclose(rcf);

  return EXIT_SUCCESS;
}
