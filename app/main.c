#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>


int exitShell(char input[]){
  if (strncmp(input, "exit",4) == 0)
    {
      return 0;
    }
}

char **getPaths(char* path, int* path_count) {
    path_count[0] = 0;
    for (int i = 0; path[i]; i++) {
        if (path[i] == ':') path_count[0]++;
    }
    path_count[0]++;
    char** filepaths = calloc(path_count[0], sizeof(char*));
    for (int i = 0; i < path_count[0]; i++) filepaths[i] = calloc(100, sizeof(char));
    int x = 0;
    int y = 0;
    for (int i = 0; path[i]; i++) {
        if (path[i] != ':') {
            filepaths[x][y++] = path[i];
        }
        else {
            //printf("%s\n", filepaths[x]);
            filepaths[x++][y] = '\0';
            y = 0;
        }
    }
    //printf("getPaths was successfull\n");
    return filepaths;
}



void echoFunction(char **args, int argc){
  
  for (int arg_idx = 1; arg_idx < argc; arg_idx++) {
        printf("%s", args[arg_idx]);
        if (arg_idx != argc - 1)
          printf(" ");
      }
      printf("\n");
  
  //printf("%s\n", args[1]);
}

int isExecutable(char *path){
  return access(path, X_OK);
}

void execute(char input[100]) {
  char *PATH = getenv("PATH");
  int *path_count = calloc(1, sizeof(int));
  char **filepaths = getPaths(PATH, path_count);
  
  for (int i = 0; i < path_count[0]; i++) {
    char *inputCopy = calloc(100, sizeof(char));
    strcpy(inputCopy, input);
    char *command = strtok(inputCopy, " ");
    char fullpath[strlen(filepaths[i]) + strlen(command)];
    
    sprintf(fullpath, "%s/%s", filepaths[i], command);
    
    if (isExecutable(fullpath) == 0) {
      char exec[strlen(filepaths[i]) + strlen(input)];
      sprintf(exec, "%s/%s", filepaths[i], input);
      // printf("%s\n", input);
      system(exec);
      return;
    }
  }
  printf("%s: command not found\n", input);
}

char *find_PATH_Exec(char input[]){
  //Set up
  char *path = getenv("PATH");
  char *command = input;
  
  char *path_copy = strdup(path);
  char *dir=strtok(path_copy,":");
  static char full_path[1024];

  if(path == NULL){
    return NULL;
  }


  while (dir!=NULL)
  {
    snprintf(full_path, sizeof(full_path),"%s/%s", dir, command);
    if(isExecutable(full_path) == 0){
      free(path_copy);
      return full_path;
    }
    dir= strtok(NULL, ":");
  }

  free(path_copy);
  return NULL;

}


void typeFunction(char input[]){
  
  // Checks for builtin functions
    if (strcmp(input+5,"echo") == 0 || strcmp(input+5,"type") == 0 || strcmp(input+5,"exit") == 0 || 
    strcmp(input+5,"pwd") == 0 || strcmp(input+5,"cd") == 0)
    {
      printf("%s is a shell builtin\n", input+5);
    }else{
      char *path = find_PATH_Exec(input+5);
      if(path){
        printf("%s is %s\n", input+5, path);
      }else{
        printf("%s: not found\n", input+5);
      }
    }
 
}

void GetDirectory(){
  char directyCurrent[1024];
  if (getcwd(directyCurrent, sizeof(directyCurrent)) != NULL){
    printf("%s\n", directyCurrent);
  }

}

void ChangeDirectory(char input[]){
  
  char *directory = input+3;
  if(strcmp(input+3,"~") == 0){
    directory = getenv("HOME");
  }
  if (chdir(directory) < 0)
  {
    printf("cd: %s: No such file or directory\n", input+3);
  }
  
}


int tokenize(char *input_str, char **args) {
  int token_num = 0;
  int token_idx = 0;
  bool token_in = false;
  bool quote_in = false;
  bool TestVar = false;
  bool dwait = false;
  bool esc_sec = false;
  char temp[1024];
  for (int input_idx = 0; input_idx < 100; input_idx++) {
    if (input_str[input_idx] == '\0') {
      temp[token_idx] = '\0';
      args[token_num] = malloc(sizeof(char) * strlen(temp));
      strcpy(args[token_num], temp);
      token_num++;
      break;
    } else if (input_str[input_idx] == '\'' && !dwait) {
      if (quote_in)
        quote_in = false;
      else
        quote_in = true;
    } else if(input_str[input_idx] == '\"'){
      if(!dwait && quote_in == false){
        dwait = true;
        
      }else{
        dwait=false;
        
      }
      }else if (input_str[input_idx] == ' ' ) {
        
      if (token_in && !quote_in && !dwait) {
        temp[token_idx] = '\0';
        token_idx = 0;
        args[token_num] = malloc(sizeof(char) * strlen(temp));
        strcpy(args[token_num], temp);
        token_num++;
        dwait = false;
        token_in = false;
        quote_in = false;
      
      } else if (token_in && quote_in && !dwait) {

        temp[token_idx] = input_str[input_idx];
        token_idx++;
      }else if ((token_in && dwait && !quote_in) || (token_in && dwait && quote_in) )
      {
        if(input_str[input_idx] == '\\'){

        char nextChar = input_str[input_idx+1];
      
        }else{
        temp[token_idx] = input_str[input_idx];
        token_idx++;
        }
      }

    }else if(input_str[input_idx] == '\\' && (!quote_in && !dwait)){
        input_idx++;
        temp[token_idx++] = input_str[input_idx];
          

    }else {
      if (token_in == 0){
        token_in = true;
      }
        if (input_str[input_idx] == '\\' && !dwait && quote_in) {
                    // Treat backslash literally
                    
                    temp[token_idx] = '\\';
                    token_idx++;
                    input_idx++;
                }else if(dwait && input_str[input_idx] == '\\'){
                  char nextChar = input_str[input_idx+1];
                  switch (nextChar)
                  {
                  case '\"':
                    temp[token_idx] = '\"';
        
                    token_idx++;
                    input_idx++; 
                    break;
                  case '\\':
                    temp[token_idx] = '\\';
                    token_idx++;
                    input_idx++; 
                    break;
                  case 'n':
                    temp[token_idx] = 'n';
                    token_idx++;
                    input_idx++; 
                    break;
                  case '$':
                    temp[token_idx] = '$';
                    token_idx++;
                    input_idx++;    
                    break;
                  default:
                    temp[token_idx] = '\\';
                    token_idx++;
                    input_idx++; 
                    break;
                  }
                  
                  
                }else{
                  temp[token_idx] = input_str[input_idx];
                  token_idx++;
                }
      
    } 
  }
  return token_num;
}


int main() {
 
  const char exit[4] = "exit";
  int argc = 0;
  char *argv[10];
  char *args[256];
  while (1)
  {

    for (int arg_idx = 0; arg_idx < argc; arg_idx++) {
      free(args[arg_idx]);
      args[arg_idx] = NULL;
    }
  
  // Uncomment this block to pass the first stage
    printf("$ ");
    fflush(stdout);

  // Wait for user input
    char input[100];
    fgets(input, 100, stdin);
    input[strlen(input) -1 ] = '\0';

    argc = tokenize(input,args);

  // Command Checks
    if(exitShell(input) == 0){
      break;
    }else if (strncmp(input, "echo", 4) == 0)
    {
      echoFunction(args, argc);
      continue;
    }else if (strncmp(input, "type", 4) == 0)
    {
      typeFunction(input);
      continue;
    }else if(strncmp(input, "pwd", 3) == 0){
      GetDirectory();
      continue;
    }else if(strncmp(input,"cd", 2) == 0){
      if (chdir(input + 3) < 0)
      {
       ChangeDirectory(input);
       continue;
      }
      continue;
    }else{
      execute(input);
      continue;

    }
    
    //Default if command not found
    printf("%s: command not found\n", input);

  }
  
  return 0;
}
