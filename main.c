#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(char **args);
int lsh_ls(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_cat(char **args);
int lsh_head(char **args);
int lsh_mkdir(char **args);
int lsh_rmdir(char **args);
int lsh_rm(char **args);
//int lsh_func(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "scd",
  "shelp",
  "sexit",
  "sls",
  "scat",
  "shead",
  "srmdir",
  "srm",
  "smkdir"
};

char *builtin_keys[] = {
  "-a",
  "-b",
  "-c",
  "-d"
};


int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit,
  &lsh_ls,
  &lsh_cat,
  &lsh_head,
  &lsh_rmdir,
  &lsh_rm,
  &lsh_mkdir
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int lsh_num_builtins_keys() {
  return sizeof(builtin_keys) / sizeof(char *);
}
int lsh_mkdir(char **args)
{
  if (args[1] == NULL){
    fprintf(stderr, "lsh: expected argument to \"mkdir\"\n");
  } else {
    if (mkdir(args[1], S_IRWXU) != 0) {
     perror("lsh");
    }
  }
  return 1;
}

/**
   @brief Bultin command: remove a directory.
   @param args List of args.  args[0] is "rmdir".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */

int lsh_rmdir(char **args)
{
    if (args[1] == NULL){
     fprintf(stderr, "lsh: expected argument to \"rmdir\"\n");
    }     
    else {
    DIR *dptr = opendir(args[1]);
    struct dirent *ds;
    char filepath[64];
    while ((ds = readdir(dptr))!= NULL)
    {
        sprintf(filepath, "%s/%s", args[1], ds->d_name);
        remove(filepath);
    }
    closedir(dptr);
    if (rmdir(args[1]) != 0) {
      perror("lsh");
    }
    }

  return 1;
}

/**
   @brief Bultin command: remove file.
   @param args List of args.  args[0] is "rm".  args[1] is the file.
   @return Always returns 1, to continue executing.
 */

int lsh_rm(char **args)
{
  if (args[1] == NULL){
    fprintf(stderr, "lsh: expected argument to \"rm\"\n");
  } else {
    if (remove(args[1]) != 0 ) {
      perror("lsh");
    }
  }
  return 1;
}

int lsh_cat(char **args)
{
    DIR *dptr;
    if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cat\"\n");
    }
    else {
        char pn;
        char currpath[64];
        pn = getwd (currpath);
        dptr = opendir(currpath);
        int c;
        FILE *file;
        file = fopen(args[1], "r");
        if (file) {
        while ((c = getc(file)) != EOF)
            putchar(c);
        fclose(file);
        }
        else fprintf(stderr, "not a file\n");
        closedir(dptr);
    }
    return 1;
}
int lsh_head(char **args)
{
    DIR *dptr;
    if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"head\"\n");
    }
    else {
        FILE *myfile;
        char content;
        int strlens = args[1];
        int max = 0;
        myfile = fopen(args[2], "r");
        content = fgetc(myfile);
        while (content != EOF){
          max++;
          if (max > strlens)
            break;
          printf ("%c", content);
          content = fgetc(myfile);
        }
        fclose(myfile);
    }
      return 1;
}

int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int lsh_ls(char **args)
{
    DIR *dptr;
    struct dirent *ds;
    
    if (args[1] == NULL) {
        char pn;
        char currpath[64];
        pn = getwd (currpath);
        dptr = opendir(currpath);
        while ((ds = readdir(dptr)) != 0)
            printf("%s\n", ds->d_name);
        closedir(dptr);
        }
    else {
        dptr = opendir(args[1]);
        if(!dptr) {
        fprintf(stderr, "not a folder\n");
        }
        else {
        while ((ds = readdir(dptr)) != 0)
            printf("%s\n", ds->d_name);
        closedir(dptr);
        }
    }
    return 1;
  /*if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"ls\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;*/
}
/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int lsh_help(char **args)
{
  int i;
  printf("Stephen Brennan's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int lsh_exit(char **args)
{
  return 0;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int lsh_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int lsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}

#define LSH_RL_BUFSIZE 1024
/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *lsh_read_line(void)
{
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    // If we hit EOF, replace it with a null character and return.
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void lsh_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("myshell>> ");
    line = lsh_read_line();
    args = lsh_split_line(line);
    status = lsh_execute(args);

    free(line);
    free(args);
  } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop.
  lsh_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}

