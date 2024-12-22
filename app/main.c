#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
char *builtin_commands[] = {"exit", "echo", "type", "pwd", "cd"};
char **shell_tokenize(const char *input, size_t *token_count) {
  if (!input)
    return NULL;
  size_t capacity = 10;
  char **tokens = malloc(capacity * sizeof(char *));
  if (!tokens)
    return NULL;
  *token_count = 0;
  const char *cursor = input;
  char buffer[1024];
  size_t buffer_index = 0;
  while (*cursor) {
    // Skip leading whitespace
    while (*cursor && isspace(*cursor)) {
      cursor++;
    }
    if (*cursor == '\0')
      break;
    char quote = '\0';
    buffer_index = 0;
    while (*cursor && (quote || !isspace(*cursor))) {
      if (*cursor == '\\') {
        cursor++;
        if (quote == '"') {
          // Inside double quotes: only escape ", \, $, and `
          // Inside double quotes: escape specific characters
          if (*cursor == '"' || *cursor == '\\' || *cursor == '$' ||
              *cursor == '`') {
            buffer[buffer_index++] = *cursor++;
          } else {
            buffer[buffer_index++] = '\\';
            if (*cursor)
              buffer[buffer_index++] = *cursor++;
          }
        } else if (*cursor == '\n') {
          // Line continuation: remove \newline pair
          cursor++;
        } else if (quote == '\0') {
          // Unquoted backslash
          if (*cursor == '\n') {
            // Line continuation: remove \newline pair
            cursor++;
          } else if (*cursor) {
            buffer[buffer_index++] = *cursor++;
          }
        } else {
          buffer[buffer_index++] = *cursor++;
          // Inside single quotes: backslashes are treated literally
          buffer[buffer_index++] = '\\';
        }
      } else if (*cursor == '"' || *cursor == '\'') {
        if (quote == *cursor) {
          // End quote
          quote = '\0';
          cursor++;
        } else if (quote == '\0') {
          // Start quote
          quote = *cursor;
          cursor++;
        } else {
          // Inside mismatched quote
          buffer[buffer_index++] = *cursor++;
        }
      } else {
        buffer[buffer_index++] = *cursor++;
      }
    }
    buffer[buffer_index] = '\0';
    if (*token_count >= capacity) {
      capacity *= 2;
      char **new_tokens = realloc(tokens, capacity * sizeof(char *));
      if (!new_tokens) {
        for (size_t i = 0; i < *token_count; i++) {
          free(tokens[i]);
        }
        free(tokens);
        return NULL;
      }
      tokens = new_tokens;
    }
    tokens[*token_count] = strdup(buffer);
    if (!tokens[*token_count]) {
      for (size_t i = 0; i < *token_count; i++) {
        free(tokens[i]);
      }
      free(tokens);
      return NULL;
    }
    (*token_count)++;
  }
  return tokens;
}
void free_tokens(char **tokens, size_t token_count) {
  if (!tokens)
    return;
  for (size_t i = 0; i < token_count; i++) {
    free(tokens[i]);
  }
  free(tokens);
}
char *find_in_path(char *command) {
  char *path = getenv("PATH");
  char *path_buf = strdup(path);
  // Find the first directory in PATH that contains the command
  char *dir = strtok(path_buf, ":");
  while (dir != NULL) {
    char command_path[100];
    sprintf(command_path, "%s/%s", dir, command);
    // Check if the command exists
    if (access(command_path, F_OK) == 0) {
      free(path_buf);
      return strdup(command_path);
    }
    dir = strtok(NULL, ":");
  }
  free(path_buf);
  return NULL;
}
void command_echo(size_t argc, char *args[]) {
  for (int i = 1; i < argc; i++) {
    printf("%s", args[i]);
    if (i < argc - 1) {
      printf(" ");
    }
  }
  printf("\n");
}
void command_type(size_t argc, char *args[]) {
  for (int i = 1; i < argc; i++) {
    for (int j = 0; j < sizeof(builtin_commands) / sizeof(builtin_commands[0]);
         j++) {
      if (strcmp(args[i], builtin_commands[j]) == 0) {
        printf("%s is a shell builtin\n", args[i]);
        return;
      }
    }
    char *command_path;
    if ((command_path = find_in_path(args[i]))) {
      printf("%s is %s\n", args[i], command_path);
      free(command_path);
      continue;
    }
    printf("%s: not found\n", args[i]);
  }
}
void command_pwd(size_t argc, char *args[]) {
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("%s\n", cwd);
  } else {
    perror("getcwd");
  }
}
void command_cd(size_t argc, char *args[]) {
  if (argc < 2) {
    fprintf(stderr, "cd: missing argument\n");
    return;
  }
  if (strcmp(args[1], "~") == 0) {
    char *home = getenv("HOME");
    if (chdir(home) != 0) {
      perror("chdir");
    }
    return;
  }
  if (chdir(args[1]) != 0) {
    printf("cd: %s: No such file or directory\n", args[1]);
  }
}
void execute_command(char *command_path, char *args[]) {
  pid_t pid = fork();
  if (pid == 0) {
    // Child process
    execv(command_path, args);
    perror("execv");
    exit(1);
  } else if (pid > 0) {
    // Parent process
    int status;
    waitpid(pid, &status, 0);
  } else {
    perror("fork");
  }
}
int main() {
  // Flush after every printf
  setbuf(stdout, NULL);
  for (;;) {
    printf("$ ");
    // Wait for user input
    char input[1024];
    fgets(input, 1024, stdin);
    if (feof(stdin)) {
      break;
    }
    // Remove the trailing newline
    input[strcspn(input, "\n")] = 0;
    // Parse the input as a space separated list of arguments
    size_t argc;
    char **args = shell_tokenize(input, &argc);
    // for (int i = 0; args[i] != NULL; i++) {
    //   printf("args[%d] = %s\n", i, args[i]);
    // }
    // Check for built-in commands
    if (args[0] != NULL) {
      switch (args[0][0]) {
      case 'c':
        if (strcmp(args[0], "cd") == 0) {
          command_cd(argc, args);
          continue;
        }
        break;
      case 'e':
        if (strcmp(args[0], "exit") == 0) {
          if (argc > 1) {
            return atoi(args[1]);
          }
          return 0;
        } else if (strcmp(args[0], "echo") == 0) {
          command_echo(argc, args);
          continue;
        }
        break;
      case 't':
        if (strcmp(args[0], "type") == 0) {
          command_type(argc, args);
          continue;
        }
        break;
      case 'p':
        if (strcmp(args[0], "pwd") == 0) {
          command_pwd(argc, args);
          continue;
        }
        break;
      }
    } else {
      continue;
    }
    // Check if command is an executable in PATH
    char *command_path;
    if ((command_path = find_in_path(args[0]))) {
      args[0] = command_path;
      execute_command(command_path, args);
      free(command_path);
      args[0] = NULL;
      continue;
    }
    free_tokens(args, argc);
    printf("%s: command not found\n", input);
  }
  return 0;
}