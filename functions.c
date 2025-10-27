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

extern int pretendent;
extern int userlist_to_remove;
extern int debug;

// variables from bashrc (after main function)
extern char *path;
extern char *etc_dir;
extern char *buildme_dir;
extern char *simplebuild_dir;
extern char *stability;

//package related variables
extern char *ver_modif;
extern char *category;
extern char *raw_package;
extern char *package;
extern char *version;
extern char *raw_patch;
extern char *patch;

extern char *filenames;
extern char *found;
extern char *sortedbyversion;

extern char *expr;
extern char *more_version;
extern char *less_version;
extern char *same_version;
extern char *exprml;

char *myregex1 = "([=<>]*)([-a-zA-Z]*)(/)([-_a-zA-Z0-9.]*)";
char *myregex2 = "([=<>]*)([-@_a-zA-Z0-9.]+)";
char *myregex3 = "([-_a-zA-Z0-9]+)(-)([0-9.]+[-a-zA-Z0-9.]*)([_ptch]*)([0-9]*)";
char *myregex4 = "([-_a-zA-Z0-9]+)(-)([0-9.]+[-a-zA-Z0-9.]*)([_ptch]*)([0-9]*)(.buildme)";
char *regexBDEP = "(BDEPEND=\")([^\"]*?)(\")";

// function to test a file type
// example: if (test_file("d", /foo/bar) == 0)
int test_file (char *file_type, char *file_pointer) {
  struct stat ft;
  int stat_status = stat(file_pointer, &ft);

  if (stat_status == -1) {
    //fprintf(stderr, "%s %s\n", file_pointer, strerror(errno));
    return 1;
  } else {
    if (strcmp(file_type, "d") == 0 && S_ISDIR(ft.st_mode)) {
      //printf("%s is a directory\n", file_pointer);
      return 0;
    }
    if (strcmp(file_type, "f") == 0 && S_ISREG(ft.st_mode)) {
      //printf("%s is a file\n", file_pointer);
      return 0;
    }
    if (strcmp(file_type, "s") == 0 && S_ISLNK(ft.st_mode)) {
      //printf("%s is a symlink\n", file_pointer);
      return 0;
    }
  }
return 1;
}


int get_expression() {
  if (debug == 1) {
    printf("function is: get expression\n");
  }

  expr = malloc(sizeof(char) * 2 + 1);
  exprml = malloc(sizeof(char) * 2 + 1);
  expr[sizeof(char) * 2] = '\0';
  exprml[sizeof(char) * 2] = '\0';
  
  // https://stackoverflow.com/a/26609503
  if (ver_modif || version) {    
    if (ver_modif && version) {
      if (strlen(ver_modif) >= 4){
        printf("ver_modif too long. current value is: %s\n", ver_modif);
        exit(1);
      }

      char *ret = NULL;
      ret = strchr(ver_modif, '=');
      if (ret != NULL){
        same_version = malloc(sizeof(char) + 1);
        sprintf(same_version, "%s", "=");
        same_version[sizeof(char)] = '\0';
        //printf("samezz %s\n", same_version);
      }

      ret = strchr(ver_modif, '>');
      if (ret != NULL){
        more_version = malloc(sizeof(char) + 1);
        sprintf(more_version, "%s", ">");
        more_version[sizeof(char)] = '\0';
      }

      ret = strchr(ver_modif, '<');
      if (ret != NULL){
        less_version = malloc(sizeof(char) + 1);
        sprintf(less_version, "%s", "<");
        less_version[sizeof(char)] = '\0';
      }

      if (more_version && less_version){
        printf("'<' and '>' can`t be in use the same time\n");
        exit(1);
      }
      
      if (more_version || less_version) {
        if (more_version) {
          sprintf(exprml, "%s", more_version);
        }
        if (less_version) {
          sprintf(exprml, "%s", less_version);          
        }
        sprintf(expr, "%s%s", exprml, same_version);
      }else{
        sprintf(expr, "%s", same_version);
      }
    }else{
      printf("PIZDEC use =package-1.2.3 or =category/package-1.2.3\n");
      exit(1);
    }
  }

  if (debug == 1){
    printf("same_version: %s\n", same_version);
    printf("less_version: %s\n", less_version);
    printf("more_version: %s\n", more_version);
    printf("exprml: %s\n", exprml);
    printf("expr: %s\n", expr);
  }

  return 0;
}

int get_variables(const char *func, char *look) {
  if (debug == 1) {
    printf("function is: get_variables\n");
    printf("called from: %s, look is: %s\n", func, look);
  }

  regex_t regex1;
  regex_t regex2;
  regex_t regex3;

  if (regcomp(&regex1, myregex1, REG_EXTENDED) != 0) {
      printf("myregex1 error\n");
      exit(1);
  }

  if (regcomp(&regex2, myregex2, REG_EXTENDED) != 0) {
      printf("myregex2 error\n");
      exit(1);
  }

  if (regcomp(&regex3, myregex3, REG_EXTENDED) != 0) {
      printf("myregex3 error\n");
      exit(1);
  }

  if (strcmp(func, "search_in_categories") == 0) {
    regmatch_t matches1[regex1.re_nsub + 1];
    if (regexec(&regex1, look, regex1.re_nsub + 1, matches1, 0) == 0){
      size_t i;
      printf("regex1\n");
      for(i = 0; i <= regex1.re_nsub; i++) {
        if(matches1[i].rm_so == -1) {
          break;
        } else {
          int length = matches1[i].rm_eo - matches1[i].rm_so;
          if (i == 1) {
            ver_modif = malloc(length * sizeof(char) + 1);
            strncpy(ver_modif, look + matches1[i].rm_so, length);
            ver_modif[length] = '\0';
            if (strlen(ver_modif) == 0){
              ver_modif = NULL;
            }
          }
          if (i == 2) {
            category = malloc(length * sizeof(char) + 1);
            strncpy(category, look + matches1[i].rm_so, length);
            category[length] = '\0';
          }
          if (i == 4) {
            raw_package = malloc(length * sizeof(char) + 1);
            strncpy(raw_package, look + matches1[i].rm_so, length);
            raw_package[length] = '\0';
          }
        }
      }

    }else{
      regmatch_t matches2[regex2.re_nsub + 1];
      if (regexec(&regex2, look, regex2.re_nsub + 1, matches2, 0) == 0) {
        printf("regex2\n");
        size_t i;
        for(i = 0; i <= regex2.re_nsub; i++) {
          //printf("i is %lu\n", i);
          if(matches2[i].rm_so == -1) {
            break;
          }else{
            int length = matches2[i].rm_eo - matches2[i].rm_so;
            if (i == 1) {
              ver_modif = malloc(length * sizeof(char) + 1);
              strncpy(ver_modif, look + matches2[i].rm_so, length);
              ver_modif[length] = '\0';
              if (strlen(ver_modif) == 0){
                ver_modif = NULL;
              }
            }
            if (i == 2) {
              raw_package = malloc(length * sizeof(char) + 1);
              strncpy(raw_package, look + matches2[i].rm_so, length);
              raw_package[length] = '\0';
            }
          }
        }
      }
    }

    regmatch_t matches3[regex3.re_nsub + 1];
    if (regexec(&regex3, look, regex3.re_nsub + 1, matches3, 0) == 0) {
      printf("regex3\n");
      size_t i;
      for(i = 0; i <= regex3.re_nsub; i++) {
        if(matches3[i].rm_so == -1) {
          break;
        }else{
          int length = matches3[i].rm_eo - matches3[i].rm_so;
          if (i == 1) {
            package = malloc(length * sizeof(char) + 1);
            strncpy(package, look + matches3[i].rm_so, length);
            package[length] = '\0';
          }
          if (i == 3) {
            version = malloc(length * sizeof(char) + 1);
            strncpy(version, look + matches3[i].rm_so, length);
            version[length] = '\0';
          }
          if (i == 4) {
            raw_patch = malloc(length * sizeof(char) + 1);
            strncpy(raw_patch, raw_package + matches3[i].rm_so, length);
            raw_patch[length] = '\0';
          }
          if (i == 5) {
            patch = malloc(length * sizeof(char) + 1);
            strncpy(patch, look + matches3[i].rm_so, length);
            patch[length] = '\0';
          }
        }
      }
    }else{
      package = strdup(raw_package);
    }
  }


  regfree(&regex1);
  regfree(&regex2);
  regfree(&regex3);

  if (debug == 1) {
    printf("ver_modif: %s\n", ver_modif);
    printf("category: %s\n", category);
    printf("raw_package: %s\n", raw_package);
    printf("package: %s\n", package);
    printf("version: %s\n", version);
    printf("raw_patch: %s\n", raw_patch);
    printf("patch: %s\n", patch);
    printf("end of get_variables()\n");
  }
  
  return 0;
}

int search_in_categories(char *look){
  //printf("argv from search_in_categories: %s\n", look);

  get_variables(__func__, look);
  get_expression();

  if (category != NULL) {
    printf("category known\n");

    FILE *cfp = fopen("./repo/profiles/categories", "r");

    if (cfp == NULL) {
      printf("no category file in profiles.\n");
      exit(1);
    }

    // TODO: real size of line
    char line[100];
    int category_found = 0;

    while (fgets(line, sizeof line, cfp) != NULL) {
      // below removes newline
      line[strcspn(line, "\n")] = 0;

      if (strcmp(category, line) == 0) {
        if (debug == 1) {
          printf("category %s found\n", category);
        }
        category_found = 1;
        break;
      }
    }

    fclose(cfp);

    if (category_found == 0) {
      printf("category %s does not exist\n", category);
      return 1;
    }

    if (userlist_to_remove == 1 ) {
      // TODO: implement.
      // line 46 in bash version
      printf("not implemented\n");
      exit(1);

    } else {
      // line 64
      char buffer[PATH_MAX+1];
      sprintf(buffer, "%s/%s/%s", buildme_dir, category, package);
      if (test_file("d", buffer) == 1) {
        return 1;
      }

      // save current directory
      char old_path[PATH_MAX+1];
      if (getcwd(old_path, sizeof(old_path)) == NULL) {
        perror("cant save current directory. getcwd() error");
        return 1;
      }

      // change directory for search
      if (chdir(buffer) != 0) {
        perror("chdir() error");
      } else {
        glob_t gstruct;
        int r;

        r = glob("*.buildme", GLOB_ERR, NULL, &gstruct);
        if (r != 0) {
          if (r == GLOB_NOMATCH) {
            printf("glob no match'n");
          } else {
            printf("glob error\n");
            exit(r);
          }
        }

        for (size_t i = 0; i < gstruct.gl_pathc; i++) {
          if(test_file("f", gstruct.gl_pathv[i]) == 0) {
            //printf("%s\n", gstruct.gl_pathv[i]);
            size_t flength = strlen(gstruct.gl_pathv[i]);
            char *filename = malloc(flength * sizeof(char));
            strcpy(filename, gstruct.gl_pathv[i]);

            if (filenames) {
              char *oldfilenames = strdup(filenames);
              free(filenames);
              sprintf(filenames, "%s %s ", oldfilenames, filename);
            } else {
              filenames = malloc(flength * sizeof(char));
              sprintf(filenames, "%s", filename);
            }
          }
        }
        void globfree(glob_t *gstruct);
      }

      if (chdir(old_path) != 0) {
        perror("chdir() error");
      }
      //line 80


      //line 90
      if (debug == 1) {
        printf("list after search: %s\n", filenames);
      }
    }


  } else {
    // line 93
    if (debug == 1) {
      printf("category not known\n");
    }
    int count = 0;

    if (userlist_to_remove == 1 ) {
      // TODO: implement.
      // line 95
      printf("not implemented\n");
      exit(1);

    } else {
      // line 122
      FILE *cfp = fopen("./repo/profiles/categories", "r");

      if (cfp == NULL) {
        printf("no category file in profiles.\n");
        exit(1);
      }

      // save current directory
      char old_path[PATH_MAX+1];
      if (getcwd(old_path, sizeof(old_path)) == NULL) {
        perror("cant save current directory. getcwd() error");
        return 1;
      }

      // change directory for search
      if (chdir(buildme_dir) != 0) {
        perror("chdir() error");
        exit(1);
      }

      // TODO: real size of variables
      char line[100];
      char testline[200];

      while (fgets(line, sizeof line, cfp) != NULL) {
        // below removes newline
        line[strcspn(line, "\n")] = 0;
        sprintf(testline, "%s/%s", line, package);

        if (test_file("d", testline) == 0) {
          // line 125
          // printf("testline %s\n", testline);
          size_t flength = strlen(testline);

          if (found) {
            char *oldfound = strdup(found);
            free(found);
            sprintf(found, "%s %s ", oldfound, testline);
          } else {
            found = malloc(flength * sizeof(char) + 1);
            sprintf(found, "%s", testline);
          }
          count = count + 1;
        }
      }

      fclose(cfp);

      if (chdir(old_path) != 0) {
        perror("chdir() error");
        exit(1);
      }
      //TODO: clean old_path here
      
      //trim trailing whitespace here
      //line 130
        
      if (count > 1) {
        printf("Sorry, we found many packages. Try to use %s\n", found);
        exit(1);
      }

      if (count == 0) {
        printf("Category for %s is not found\n", raw_package);
        exit(1);
      }

      printf("found is \"%s\"\n", found);


      //line 143
      get_variables(__func__,found);

      if (debug == 1) {
        printf("ver_modif: %s\n", ver_modif);
        printf("category: %s\n", category);
        printf("package: %s\n", package);
      }


      char buffer[PATH_MAX+1];
      sprintf(buffer, "%s/%s/%s", buildme_dir, category, package);
      if (test_file("d", buffer) == 1) {
        return 1;
      }

      // save current directory
      if (getcwd(old_path, sizeof(old_path)) == NULL) {
        perror("cant save current directory. getcwd() error");
        return 1;
      }

      // change directory for search
      if (chdir(buffer) != 0) {
        perror("chdir() error");
        exit(1);

      //
      } else {
        glob_t gstruct;
        int r;

        r = glob("*.buildme", GLOB_ERR, NULL, &gstruct);
        if (r != 0) {
          if (r == GLOB_NOMATCH) {
            printf("glob no match'n");
          } else {
            printf("glob error\n");
            exit(r);
          }
        }

        for (size_t i = 0; i < gstruct.gl_pathc; i++) {
          if(test_file("f", gstruct.gl_pathv[i]) == 0) {
            //printf("%s\n", gstruct.gl_pathv[i]);
            size_t flength = strlen(gstruct.gl_pathv[i]);
            char *filename = malloc(flength * sizeof(char) + 1);
            strcpy(filename, gstruct.gl_pathv[i]);

            if (filenames) {
              char *oldfilenames = strdup(filenames);
              free(filenames);
              sprintf(filenames, "%s %s ", oldfilenames, filename);
            } else {
              filenames = malloc(flength * sizeof(char) + 1);
              sprintf(filenames, "%s", filename);
            }
          }
        }
        void globfree(glob_t *gstruct);
      }

      


      if (chdir(old_path) != 0) {
        perror("chdir() error");
        exit(1);
      }
    }

    if (debug == 1) {
      printf("list after search: %s\n", filenames);
    }
  }

  // check version
  // line 170

  if (ver_modif != NULL) {
    printf("not implemented\n");
    exit(1);
  } else {
    sortedbyversion = strdup(filenames);
  }

  if (debug == 1) {
    printf("sorted by version: %s\n", sortedbyversion);
  }

  // check stability
  // line 203

  if (userlist_to_remove == 1 ) {
    // TODO: implement.
    // line 206
    printf("not implemented\n");
    exit(1);
  } else {
    char* f;
    char* rest = sortedbyversion;
    while ((f = strtok_r(rest, " ", &rest))) {
      if (debug == 1) {
        printf("f: %s\n", f);
      }
      

    }
  }

  // reset global variables

  sortedbyversion = NULL;
  ver_modif = NULL;
  category = NULL;
  package = NULL;
  raw_package = NULL;
  version = NULL;
  found = NULL;
  filenames = NULL;

  return 0;
}


int get_dep(){

  char buildme_path[PATH_MAX+1];

  sprintf(buildme_path, "%s/%s/%s/%s-%s%s.buildme", buildme_dir, category, package, package, version, raw_patch);

  printf("buildme_path is %s\n", buildme_path);

  // parse buildme file
  char *key, *value;
  FILE *ptr = fopen(buildme_path, "r");

  if (ptr == NULL) {
    printf("no such file.\n");
    return 1;
  }

  

  
  return 0;
}
