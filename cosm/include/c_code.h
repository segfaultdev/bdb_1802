#ifndef __COSM_CODE_H__
#define __COSM_CODE_H__

#include <stdint.h>

typedef struct c_code_t c_code_t;

struct c_code_t {
  char c_name[5], u_name[6];
  uint8_t code;
  
  enum {
    c_param_none,
    
    c_param_register,
    c_param_port,
    
    c_param_byte,
    c_param_pair,
  } param;
};

extern const c_code_t c_code_array[];
extern const int c_code_count;

#endif
