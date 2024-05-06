#ifndef __COSMAC_H__
#define __COSMAC_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct cosmac_t cosmac_t;

struct cosmac_t {
  uint16_t r[16];
  uint8_t d, t, b;
  
  uint8_t p : 4;
  uint8_t x : 4;
  
  uint8_t i : 4;
  uint8_t n : 4;
  
  bool df, ie;
  bool ef[4], q;
  
  enum {
    s0_fetch,
    
    s1_reset,
    s1_init,
    s1_execute_1,
    s1_execute_2,
    s1_wait,
    
    s2_dma,
    
    s3_int,
  } s;
  
  bool dma_line, irq_line;
  
  void    (*write)(cosmac_t *, uint16_t, uint8_t);
  uint8_t (*read)(cosmac_t *, uint16_t);
  
  void    (*write_io)(cosmac_t *, uint8_t, uint8_t);
  uint8_t (*read_io)(cosmac_t *, uint8_t);
};

void cosmac_init(cosmac_t *cosmac);
void cosmac_tick(cosmac_t *cosmac);

#endif
