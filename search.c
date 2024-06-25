#include <regex.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include <unistd.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "functions.h"

int pretendent = 0;
int userlist_to_remove = 0;
int debug = 0;

// variables from bashrc (after main function)
char *path = NULL;
char *etc_dir = NULL;
char *buildme_dir = NULL;
char *simplebuild_dir = NULL;
char *stability = NULL;

//package related variables
char *ver_modif = NULL;
char *category = NULL;
char *raw_package = NULL;
char *package = NULL;
char *version = NULL;
char *raw_patch = NULL;
char *patch = NULL;

// expression related
char *expr = NULL;
char *more_version = NULL;
char *less_version = NULL;
char *same_version = NULL;
char *exprml = NULL;


char *filenames = NULL;
char *found = NULL;

// https://riptutorial.com/c/example/3250/calling-a-function-from-another-c-file


int main(int argc, char **argv){

  //get bashrc path
  char *bashrc_path = getenv("BASH_ENV");

  // parse bashrc
  char *key, *value;
  FILE *ptr = fopen(bashrc_path, "r");

  if (ptr == NULL) {
    printf("no such file.\n");
    return 1;
  }

  while (fscanf(ptr, "%m[^=]=\"%m[^\"]\"\n", &key, &value) == 2) {
    //printf ("%s=\"\%s\"\n", key, value);
    if (strcmp(key, "PATH") == 0) {
      size_t length = strlen(value);
      path = malloc(length * sizeof(char) + 1);
      strcpy(path, value);
      path[length] = '\0';
    }
    if (strcmp(key, "ETC_DIR") == 0) {
      size_t length = strlen(value);
      etc_dir = malloc(length * sizeof(char) + 1);
      strcpy(etc_dir, value);
      etc_dir[length] = '\0';
    }
    if (strcmp(key, "BUILDME_DIR") == 0) {
      size_t length = strlen(value);
      buildme_dir = malloc(length * sizeof(char) +1);
      strcpy(buildme_dir, value);
      buildme_dir[length] = '\0';
    }
    if (strcmp(key, "SIMPLEBUILD_DIR") == 0) {
      size_t length = strlen(value);
      simplebuild_dir = malloc(length * sizeof(char) + 1);
      strcpy(simplebuild_dir, value);
      simplebuild_dir[length] = '\0';
    }
    if (strcmp(key, "STABILITY") == 0) {
      size_t length = strlen(value);
      stability = malloc(length * sizeof(char) + 1);
      strcpy(stability, value);
      stability[length] = '\0';
    }
    if (strcmp(key, "PRETENDENT") == 0) {
      if (strcmp(value, "true") == 0) {
        pretendent = 1;
      }
    }
    if (strcmp(key, "REMOVE") == 0) {
      if (strcmp(value, "true") == 0) {
        userlist_to_remove = 1;
      }
    }
    if (strcmp(key, "DEBUG") == 0) {
      if (strcmp(value, "true") == 0) {
        debug = 1;
      }
    }
      
    free(key);
    free(value);
  }
  fclose(ptr);

  if (debug == 1) {
    printf ("bashrc_path is %s\n", bashrc_path);
    printf ("path %s\n", path);
    printf ("etc_dir %s\n", etc_dir);
    printf ("buildme_dir %s\n", buildme_dir);
    printf ("simplebuild_dir %s\n", simplebuild_dir);
    printf ("stability %s\n", stability);
    printf ("pretendent %d\n", pretendent);
    printf ("userlist_to_remove %d\n", userlist_to_remove);
    printf ("debug %d\n", debug);
  }



  if (argc == 1){
    printf("required at least one parametr\n");
    return 1;
  }

  for (int a = 1; a < argc; a++) {
    printf("argv: %s\n", argv[a]);

    search_in_categories(argv[a]);

    get_dep();
  }

  return 0;
}

