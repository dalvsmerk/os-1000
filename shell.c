#include "stdio.h"
#include "string.h"
#include "user.h"

void cmd_hello(void);
void cmd_help(void);
void cmd_exit(void);

void main(void) {
  printf("os1000-shell v0.0.1\n");
  printf("see 'help' command for list of available "
         "commands\n\n");

  while (1) {
    printf("$ ");

    char cmd[128];

    for (int i = 0; i < 128; i++) {
      char c = getchar();

      if (c == '\r') {
        putchar('\n');
        cmd[i] = '\0'; // to be used with strcmp
        break;
      }

      cmd[i] = c;
      putchar(c);
    }

    if (strcmp("hello", cmd) == 0) {
      cmd_hello();
      continue;
    }

    if (strcmp("help", cmd) == 0) {
      cmd_help();
      continue;
    }

    if (strcmp("exit", cmd) == 0) {
      cmd_exit();
    }

    if (strcmp("", cmd) == 0) {
      // just newline
      continue;
    }

    printf("unrecognized command '%s', see 'help' command\n", cmd);
  }
}

void cmd_hello(void) { printf("hello from userland!\n"); }

void cmd_help(void) {
  printf("help       print list of available commands\n");
  printf("hello      print welcome message\n");
  printf("exit       to exit shell\n");
}

void cmd_exit(void) { exit(); }
