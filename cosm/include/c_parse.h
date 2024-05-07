#ifndef __COSM_PARSE_H__
#define __COSM_PARSE_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define C_DELIM " \t\r\n"

typedef struct c_refer_t c_refer_t;
typedef struct c_label_t c_label_t;

struct c_refer_t {
  char l_name[31], r_name[31];
  uint16_t address, index;
  
  bool l_shift, r_shift;
  
  bool is_branch;
  int width;
};

struct c_label_t {
  char name[31];
  uint16_t pair;
};

extern uint8_t *c_byte_array;
extern int c_byte_count, c_byte_limit;

extern c_refer_t *c_refer_array;
extern int c_refer_count;

extern c_label_t *c_label_array;
extern int c_label_count;

extern uint16_t c_address;

void c_byte_insert(uint8_t byte);

void c_refer_insert(c_refer_t refer);
void c_label_insert(c_label_t label);

const c_label_t *c_label_find(const char *name);

uint16_t c_parse_pair(const char *word, int width);
void     c_parse_data(const char *word, int width, bool is_branch);
void     c_parse_line(const char *word, bool use_u);
void     c_parse_file(FILE *file, bool use_u);

void c_save(FILE *file);

#endif
