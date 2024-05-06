#include <c_parse.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void help(const char *arg0, bool in_depth) {
  fprintf(stderr, "cosm r01, by segfaultdev (2024).\n");
  
  fprintf(stderr, "\nUsage:\n");
  fprintf(stderr, "  %s [...] <I_FILE> -O <O_FILE>\n", arg0);
  fprintf(stderr, "  %s -H\n", arg0);
  
  if (!in_depth) {
    return;
  }
  
  fprintf(stderr, "\nOptions:\n");
  fprintf(stderr, "  -H            Print this help.\n");
  fprintf(stderr, "  -O <O_FILE>   Write to this file.\n");
  fprintf(stderr, "  -L <ADDRESS>  Set the loading address.\n");
  fprintf(stderr, "  -U            Use updated opcodes (default).\n");
  fprintf(stderr, "  -C            Use conventional opcodes.\n");
}

int main(int argc, const char *argv[]) {
  FILE *i_file = NULL, *o_file = NULL;
  bool use_u = true;
  
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-H") || !strcmp(argv[i], "-h")) {
      help(argv[0], true);
      exit(0);
    } else if (!strcmp(argv[i], "-O") || !strcmp(argv[i], "-o")) {
      if (++i == argc) {
        fprintf(stderr, "cosm: expected argument after '-O'.\n");
        exit(1);
      }
      
      if (!strcmp(argv[i], "-")) {
        o_file = stdout;
      } else {
        o_file = fopen(argv[i], "wb");
        
        if (o_file == NULL) {
          fprintf(stderr, "cosm: could not write file '%s'.\n", argv[i]);
          exit(1);
        }
      }
    } else if (!strcmp(argv[i], "-L") || !strcmp(argv[i], "-l")) {
      if (++i == argc) {
        fprintf(stderr, "cosm: expected argument after '-L'.\n");
        exit(1);
      }
      
      c_address = strtoul(argv[i], NULL, 0);
    } else if (!strcmp(argv[i], "-U") || !strcmp(argv[i], "-u")) {
      use_u = true;
    } else if (!strcmp(argv[i], "-C") || !strcmp(argv[i], "-c")) {
      use_u = false;
    } else {
      if (!strcmp(argv[i], "-")) {
        i_file = stdin;
      } else {
        i_file = fopen(argv[i], "rb");
        
        if (i_file == NULL) {
          fprintf(stderr, "cosm: could not read file '%s'.\n", argv[i]);
          exit(1);
        }
      }
    }
  }
  
  if (i_file == NULL || o_file == NULL) {
    help(argv[0], false);
    exit(1);
  }
  
  c_parse_file(i_file, use_u);
  c_save(o_file);
  
  if (i_file != stdin) {
    fclose(i_file);
  }
  
  if (o_file != stdout) {
    fclose(o_file);
  }
  
  return 0;
}
