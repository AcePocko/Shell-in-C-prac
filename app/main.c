#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#define INPUTLEN 4096
#define PATHLEN 4096
#define ARGNUM 256
typedef enum builtin_cmd { usrcmd = 0, cd, ex, echo, pwd, type } shCmd;
void get_command(char *cmd_str, int maxlen) {
  int ch = 0;
  int len = 0;
  while (((ch = getchar()) != EOF && ch != '\n') && len < maxlen) {
    cmd_str[len] = ch;
    len++;
  }
  cmd_str[len] = '\0';
}
/* Expands argument list with a new token. */
void add_token(char *token, char **args, int token_num, int token_len) {
  token[token_len] = '\0';
  args[token_num] = malloc(sizeof(char) * token_len);
  strcpy(args[token_num], token);
}
/* Tokenizes the command line input. */
int tokenize(char *input_str, char **args, int maxlen) {
  int token_num = 0;
  int token_idx = 0;
  bool token_in = false;
  bool single_quote_in = false;
  bool double_quote_in = false;
  bool esc_sec = false;
  char temp[1024];
  for (int input_idx = 0; input_idx < maxlen; input_idx++) {
    char ch = input_str[input_idx];
    if (ch == '\0') {
      add_token(temp, args, token_num, token_idx);
      token_num++;
      break;
    }
    switch (ch) {
    case '\\':
      if (!single_quote_in && !double_quote_in) {
        if (!esc_sec) {
          esc_sec = true;
        } else {
          if (token_in == 0)
            token_in = true;
          temp[token_idx] = ch;
          token_idx++;
          esc_sec = false;
        }
      } else {
        temp[token_idx] = ch;
        token_idx++;
      }
      break;
    case '\'':
      if (esc_sec) {
        if (token_in == 0)
          token_in = true;
        temp[token_idx] = ch;
        token_idx++;
        esc_sec = false;
        break;
      }
      if (double_quote_in) {
        temp[token_idx] = ch;
        token_idx++;
      } else {
        if (single_quote_in) {
          single_quote_in = false;
        } else {
          single_quote_in = true;
          token_in = true;
        }
      }
      break;
    case '"':
      if (esc_sec) {
        if (token_in == 0)
          token_in = true;
        temp[token_idx] = ch;
        token_idx++;
        esc_sec = false;
        break;
      }
      if (double_quote_in) {
        double_quote_in = false;
      if (single_quote_in) {
        temp[token_idx] = ch;
        token_idx++;
      } else {
        double_quote_in = true;
        token_in = true;
        if (double_quote_in) {
          double_quote_in = false;
        } else {
          double_quote_in = true;
          token_in = true;
        }
      }
      break;
    case ' ':
Expand 98 lines
      strcpy(env_path, getenv("PATH")); // Environment path
      if (cmd_name > usrcmd) {
        printf("%s is a shell builtin\n", args[1]);
        fflush(stdout);
      } else {
        if ((parse_path(env_path, args[1], cmd_path)) != -1)
        if ((parse_path(env_path, args[1], cmd_path)) != -1) {
          printf("%s is %s\n", args[1], cmd_path);
        else
          fflush(stdout);
        } else {
          printf("%s: not found\n", args[1]);
          fflush(stdout);
        }
      }
    } break;
    case cd:
      if (!strcmp(args[1], "~"))
        chdir(home_path);
      else if (chdir(args[1]) != 0)
      else if (chdir(args[1]) != 0) {
        printf("%s: No such file or directory\n", args[1]);
        fflush(stdout);
      }
      break;
    case ex:
      if (args[1] != NULL) {
        int exit_code = atoi(args[1]);
        exit(exit_code);
      } else {
        exit(0);
      }
      break;
    case echo:
      for (int arg_idx = 1; arg_idx < num_args; arg_idx++) {
        printf("%s", args[arg_idx]);
        if (arg_idx != num_args - 1)
        fflush(stdout);
        if (arg_idx != num_args - 1) {
          printf(" ");
          fflush(stdout);
        }
      }
      printf("\n");
      fflush(stdout);
      break;
    case pwd:
      if (getcwd(curr_path, PATHLEN) != NULL)
      if (getcwd(curr_path, PATHLEN) != NULL) {
        printf("%s\n", curr_path);
        fflush(stdout);
      }
      break;
    case usrcmd:
      strcpy(env_path, getenv("PATH"));
      if ((parse_path(env_path, args[0], cmd_path)) != -1) {
        pid_t pid = fork();
        if (pid == 0) {
          int ret = execvp(cmd_path, args);
          exit(ret);
        }
        int status = 0;
        while ((pid = wait(&status)) > 0)
          ;
      } else {
        printf("%s: not found\n", user_input);
        fflush(stdout);
      }
      break;
    }
    // Freeing the memory of previous tokens after the command is analyzed
    for (int arg_idx = 0; arg_idx < num_args; arg_idx++) {
      free(args[arg_idx]);
      args[arg_idx] = NULL;
    }
  }
  return 0;
}