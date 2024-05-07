#include <c_parse.h>
#include <stdbool.h>
#include <c_code.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

uint8_t *c_byte_array = 0;
int c_byte_count = 0, c_byte_limit = 0;

c_refer_t *c_refer_array = 0;
int c_refer_count = 0, c_refer_limit = 0;

c_label_t *c_label_array = 0;
int c_label_count = 0, c_label_limit = 0;

uint16_t c_address = 0;

void c_byte_insert(uint8_t byte) {
  if (c_byte_count == c_byte_limit) {
    c_byte_array = realloc(c_byte_array, (c_byte_limit += 64) * sizeof(uint8_t));
  }
  
  c_byte_array[c_byte_count++] = byte;
  c_address++;
}

void c_refer_insert(c_refer_t refer) {
  if (c_refer_count == c_refer_limit) {
    c_refer_array = realloc(c_refer_array, (c_refer_limit += 16) * sizeof(c_refer_t));
  }
  
  c_refer_array[c_refer_count++] = refer;
}

void c_label_insert(c_label_t label) {
  if (c_label_find(label.name) != NULL) {
    fprintf(stderr, "cosm: redefined label '%s'.\n", label.name);
    exit(1);
  }
  
  if (c_label_count == c_label_limit) {
    c_label_array = realloc(c_label_array, (c_label_limit += 8) * sizeof(c_label_t));
  }
  
  c_label_array[c_label_count++] = label;
}

const c_label_t *c_label_find(const char *name) {
  for (int i = 0; i < c_label_count; i++) {
    if (strcmp(c_label_array[i].name, name)) {
      continue;
    }
    
    return c_label_array + i;
  }
  
  return NULL;
}

uint16_t c_parse_pair(const char *word, int width) {
  const char *end_pointer;
  const uint16_t pair = strtoul(word, (char **)(&end_pointer), 0);
  
  if (*end_pointer != '\0') {
    fprintf(stderr, "cosm: unknown %s '%s' found.\n", ((width == 2) ? "pair" : "byte"), word);
    exit(1);
  }
  
  return pair;
}

void c_parse_data(const char *word, int width, bool is_branch) {
  const char *end_pointer;
  const uint16_t pair = strtoul(word, (char **)(&end_pointer), 0);
  
  if (*end_pointer != '\0') {
    c_refer_t refer = {"", "", c_address, c_byte_count, false, false, is_branch, width};
    
    if (*word == '^') {
      refer.l_shift = true;
      word++;
    }
    
    strncat(refer.l_name, word, 30);
    
    if ((word = strtok(NULL, C_DELIM)) != NULL && !strcmp(word, "-")) {
      if ((word = strtok(NULL, C_DELIM)) == NULL) {
        fprintf(stderr, "cosm: expected parameter after '-'.\n");
        exit(1);
      }
      
      if (*word == '^') {
        refer.r_shift = true;
        word++;
      }
      
      strncat(refer.r_name, word, 30);
    }
    
    c_refer_insert(refer);
  }
  
  if (width == 2) {
    c_byte_insert((uint8_t)(pair >> 8));
  } else if (*end_pointer == '\0') {
    if ((pair >> 8) != (is_branch ? (c_address >> 8) : 0)) {
      fprintf(stderr, "cosm: (warning) '%s' truncated to a byte.\n", word);
    }
  }
  
  c_byte_insert((uint8_t)(pair >> 0));
}

void c_parse_line(const char *word, bool use_u) {
  if (word == NULL) {
    return;
  }
  
  int length = strlen(word);
  
  if (word[0] == '#' || length == 0) {
    return;
  }
  
  if (word[length - 1] == ':') {
    if (length > 31) {
      length = 31;
    }
    
    c_label_t label = {"", c_address};
    strncat(label.name, word, length - 1);
    
    c_label_insert(label);
    return;
  }
  
  if (!strcmp(word, ".byte") || !strcmp(word, ".pair")) {
    const int width = ((word[1] == 'p') ? 2 : 1);
    
    while ((word = strtok(NULL, C_DELIM)) != NULL) {
      c_parse_data(word, width, false);
    }
    
    return;
  }
  
  if (!strcmp(word, ".define")) {
    if ((word = strtok(NULL, C_DELIM)) == NULL) {
      fprintf(stderr, "cosm: expected parameter after '.define'.\n");
      exit(1);
    }
    
    c_label_t label = {"", 0};
    strncat(label.name, word, 30);
    
    if ((word = strtok(NULL, C_DELIM)) == NULL) {
      fprintf(stderr, "cosm: expected parameter after '%s'.\n", label.name);
      exit(1);
    }
    
    label.pair = c_parse_pair(word, 2);
    
    c_label_insert(label);
    return;
  }
  
  if (!strcmp(word, ".include")) {
    strtok(NULL, "\"");
    
    if ((word = strtok(NULL, "\"")) == NULL) {
      fprintf(stderr, "cosm: expected string after '.include'.\n");
      exit(1);
    }
    
    FILE *file = fopen(word, "rb");
    
    if (file == NULL) {
      fprintf(stderr, "cosm: could not read file '%s'.\n", word);
      exit(1);
    }
    
    c_parse_file(file, use_u);
    fclose(file);
    
    return;
  }
  
  if (!strcmp(word, ".load")) {
    if ((word = strtok(NULL, C_DELIM)) == NULL) {
      fprintf(stderr, "cosm: expected parameter after '.load'.\n");
      exit(1);
    }
    
    c_address = c_parse_pair(word, 2);
    return;
  }
  
  if (!strcmp(word, ".text")) {
    strtok(NULL, "\"");
    
    if ((word = strtok(NULL, "\"")) == NULL) {
      fprintf(stderr, "cosm: expected string after '.text'.\n");
      exit(1);
    }
    
    for (int i = 0; word[i]; i++) {
      c_byte_insert(word[i]);
    }
    
    return;
  }
  
  for (int i = 0; i < c_code_count; i++) {
    c_code_t code = c_code_array[i];
    const char *name = (use_u ? code.u_name : code.c_name);
    
    if (strcmp(word, name)) {
      continue;
    }
    
    if (code.param != c_param_none && (word = strtok(NULL, C_DELIM)) == NULL) {
      fprintf(stderr, "cosm: expected parameter after '%s'.\n", name);
      exit(1);
    }
    
    if (code.param == c_param_register || code.param == c_param_port) {
      const char *register_array[] = {
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        "r8", "r9", "ra", "rb", "rc", "rd", "re", "rf",
      };
      
      const char *port_array[] = {
        "p1", "p2", "p3", "p4", "p5", "p6", "p7",
      };
      
      const char **array = ((code.param == c_param_port) ? port_array : register_array);
      bool found = false;
      
      for (int j = 0; j < 16; j++) {
        if (strcmp(word, array[j])) {
          continue;
        }
        
        code.code += j;
        found = true;
        
        break;
      }
      
      if (!found) {
        fprintf(stderr, "cosm: unknown %s '%s' found.\n", ((code.param == c_param_port) ? "port" : "register"), word);
        exit(1);
      }
    }
    
    c_byte_insert(code.code);
    
    if (code.param == c_param_byte) {
      c_parse_data(word, 1, ((code.code >> 4) == 3));
    }
    
    if (code.param == c_param_pair) {
      c_parse_data(word, 2, false);
    }
    
    return;
  }
  
  fprintf(stderr, "cosm: unknown word '%s' found.\n", word);
  exit(1);
}

void c_parse_file(FILE *file, bool use_u) {
  char buffer[224];
  
  while (!feof(file)) {
    int c, length = 0;
    
    while ((c = fgetc(file)) != EOF) {
      if (c == '\n') {
        break;
      }
      
      if (isspace(c)) {
        if (c == '\r' || length == 0) {
          continue;
        }
      }
      
      buffer[length++] = c;
    }
    
    buffer[length] = '\0';
    c_parse_line(strtok(buffer, C_DELIM), use_u);
  }
}

void c_save(FILE *file) {
  for (int i = 0; i < c_refer_count; i++) {
    c_refer_t refer = c_refer_array[i];
    uint16_t l_pair, r_pair;
    
    if (refer.l_name[0] == '\0') {
      l_pair = 0;
    } else if (!strcmp(refer.l_name, "$")) {
      l_pair = refer.address;
    } else if (!strcmp(refer.l_name, "%")) {
      l_pair = refer.index;
    } else {
      const c_label_t *l_label = c_label_find(refer.l_name);
      
      if (l_label == NULL) {
        fprintf(stderr, "cosm: unknown label '%s' found.\n", refer.l_name);
        exit(1);
      }
      
      l_pair = l_label->pair;
    }
    
    if (refer.r_name[0] == '\0') {
      r_pair = 0;
    } else if (!strcmp(refer.r_name, "$")) {
      r_pair = refer.address;
    } else if (!strcmp(refer.r_name, "%")) {
      r_pair = refer.index;
    } else {
      const c_label_t *r_label = c_label_find(refer.r_name);
      
      if (r_label == NULL) {
        fprintf(stderr, "cosm: unknown label '%s' found.\n", refer.r_name);
        exit(1);
      }
      
      r_pair = r_label->pair;
    }
    
    if (refer.l_shift) {
      l_pair = (l_pair >> 8);
    }
    
    if (refer.r_shift) {
      r_pair = (r_pair >> 8);
    }
    
    if (r_pair > l_pair) {
      fprintf(stderr, "cosm: (warning) '%s%s - %s%s' underflowed.\n", (refer.l_shift ? "^" : ""), refer.l_name, (refer.r_shift ? "^" : ""), refer.r_name);
    }
    
    const uint16_t pair = l_pair - r_pair;
    
    if (refer.width == 2) {
      c_byte_array[refer.index++] = (uint8_t)(pair >> 8);
    } else if ((pair >> 8) != (refer.is_branch ? (refer.address >> 8) : 0)) {
      fprintf(stderr, "cosm: (warning) '%s%s - %s%s' truncated to a byte.\n", (refer.l_shift ? "^" : ""), refer.l_name, (refer.r_shift ? "^" : ""), refer.r_name);
    }
    
    c_byte_array[refer.index] = (uint8_t)(pair >> 0);
  }
  
  rewind(file);
  fwrite(c_byte_array, sizeof(uint8_t), c_byte_count, file);
}
