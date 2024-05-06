#include <stdbool.h>
#include <cosmac.h>
#include <stdint.h>
#include <string.h>

void cosmac_init(cosmac_t *cosmac) {
  cosmac->dma_line = false;
  cosmac->irq_line = false;
  
  cosmac->s = s1_reset;
}

static void tick_s1(cosmac_t *cosmac) {
  if (cosmac->i == 0 && cosmac->n == 0) {
    cosmac->s = s1_wait;
    return;
  }
  
  if (cosmac->i == 0) {
    cosmac->d = cosmac->read(cosmac, cosmac->r[cosmac->n]);
  }
  
  if (cosmac->i == 1) {
    cosmac->r[cosmac->n]++;
  }
  
  if (cosmac->i == 2) {
    cosmac->r[cosmac->n]--;
  }
  
  if (cosmac->i == 3) {
    const uint8_t l = cosmac->n & 7;
    bool c = true;
    
    if (l == 1) {
      c = cosmac->q;
    }
    
    if (l == 2) {
      c = (cosmac->d == 0);
    }
    
    if (l == 3) {
      c = cosmac->df;
    }
    
    if (l >= 4) {
      c = cosmac->ef[l - 4];
    }
    
    if (cosmac->n >= 8) {
      c = !c;
    }
    
    if (c) {
      const uint8_t b = cosmac->read(cosmac, cosmac->r[cosmac->p]);
      
      cosmac->r[cosmac->p] &= 65280;
      cosmac->r[cosmac->p] |= b;
    } else {
      cosmac->r[cosmac->p]++;
    }
  }
  
  if (cosmac->i == 4) {
    cosmac->d = cosmac->read(cosmac, cosmac->r[cosmac->n]++);
  }
  
  if (cosmac->i == 5) {
    cosmac->write(cosmac, cosmac->r[cosmac->n], cosmac->d);
  }
  
  if (cosmac->i == 6 && cosmac->n == 0) {
    cosmac->r[cosmac->x]++;
    return;
  }
  
  if (cosmac->i == 6) {
    if (cosmac->n >= 8) {
      cosmac->d = cosmac->read_io(cosmac, cosmac->n & 7);
    } else {
      cosmac->write_io(cosmac, cosmac->n & 7, cosmac->d);
    }
  }
  
  if (cosmac->i == 7 && cosmac->n <= 1) {
    const uint8_t b = cosmac->read(cosmac, cosmac->r[cosmac->x]++);
    
    cosmac->p = (b >> 0) & 15;
    cosmac->x = (b >> 4) & 15;
    
    cosmac->ie = (cosmac->n == 0);
    return;
  }
  
  if (cosmac->i == 7 && cosmac->n == 2) {
    cosmac->d = cosmac->read(cosmac, cosmac->r[cosmac->x]++);
    return;
  }
  
  if (cosmac->i == 7 && cosmac->n == 3) {
    cosmac->write(cosmac, cosmac->r[cosmac->x]--, cosmac->d);
    return;
  }
  
  if (cosmac->i == 7 && cosmac->n == 6) {
    cosmac->df = ((cosmac->d & 1) != 0);
    cosmac->d = (cosmac->d >> 1) | (cosmac->df ? 128 : 0);
    
    return;
  }
  
  if (cosmac->i == 7 && cosmac->n == 8) {
    cosmac->write(cosmac, cosmac->r[cosmac->x], cosmac->t);
    return;
  }
  
  if (cosmac->i == 7 && cosmac->n == 9) {
    cosmac->t = cosmac->p | (cosmac->x << 4);
    cosmac->write(cosmac, cosmac->r[2]--, cosmac->t);
    
    cosmac->x = cosmac->p;
    return;
  }
  
  if (cosmac->i == 7 && cosmac->n == 10) {
    cosmac->q = false;
    return;
  }
  
  if (cosmac->i == 7 && cosmac->n == 11) {
    cosmac->q = true;
    return;
  }
  
  if (cosmac->i == 7 && cosmac->n == 14) {
    cosmac->df = ((cosmac->d & 128) != 0);
    cosmac->d = (cosmac->d << 1) | (cosmac->df ? 1 : 0);
    
    return;
  }
  
  if (cosmac->i == 8) {
    cosmac->d = (cosmac->r[cosmac->n] >> 0) & 255;
  }
  
  if (cosmac->i == 9) {
    cosmac->d = (cosmac->r[cosmac->n] >> 8) & 255;
  }
  
  if (cosmac->i == 10) {
    cosmac->r[cosmac->n] &= 65280;
    cosmac->r[cosmac->n] |= cosmac->d;
  }
  
  if (cosmac->i == 11) {
    cosmac->r[cosmac->n] &= 255;
    cosmac->r[cosmac->n] |= ((uint16_t)(cosmac->d) << 8);
  }
  
  if (cosmac->i == 12) {
    if ((cosmac->n & 7) >= 4) {
      cosmac->b = (cosmac->r[cosmac->p] >> 8) & 255;
    } else {
      cosmac->b = cosmac->read(cosmac, cosmac->r[cosmac->p]++);
    }
  }
  
  if (cosmac->i == 13) {
    cosmac->p = cosmac->n;
  }
  
  if (cosmac->i == 14) {
    cosmac->x = cosmac->n;
  }
  
  if (cosmac->i == 15 && cosmac->n == 0) {
    cosmac->d = cosmac->read(cosmac, cosmac->r[cosmac->x]);
    return;
  }
  
  if (cosmac->i == 15 && cosmac->n == 6) {
    cosmac->df = ((cosmac->d & 1) != 0);
    cosmac->d = (cosmac->d >> 1);
    
    return;
  }
  
  if (cosmac->i == 15 && cosmac->n == 14) {
    cosmac->df = ((cosmac->d & 128) != 0);
    cosmac->d = (cosmac->d << 1);
    
    return;
  }
  
  if (cosmac->i == 7 || cosmac->i == 15) {
    const uint8_t l = cosmac->n & 7;
    uint16_t b, r;
    
    if (cosmac->n >= 8) {
      b = cosmac->read(cosmac, cosmac->r[cosmac->p]++);
    } else {
      b = cosmac->read(cosmac, cosmac->r[cosmac->x]);
    }
    
    if (l == 1) {
      r = b | (uint16_t)(cosmac->d);
    }
    
    if (l == 2) {
      r = b & (uint16_t)(cosmac->d);
    }
    
    if (l == 3) {
      r = b ^ (uint16_t)(cosmac->d);
    }
    
    if (l == 4) {
      r = b + (uint16_t)(cosmac->d);
    }
    
    if (l == 5) {
      r = b + (uint16_t)(cosmac->d ^ 255);
    }
    
    if (l == 7) {
      r = (b ^ 255) + (uint16_t)(cosmac->d);
    }
    
    if (l >= 4) {
      if (cosmac->i == 7) {
        r += (cosmac->df ? 1 : 0);
      }
      
      if (cosmac->i == 15) {
        r += ((l != 0) ? 1 : 0);
      }
      
      cosmac->df = (r >= 256);
    }
    
    cosmac->d = r & 255;
  }
}

static void tick_s2(cosmac_t *cosmac) {
  const uint8_t l = cosmac->n & 3;
  
  uint8_t b;
  bool c = true;
  
  if ((cosmac->n & 7) >= 4) {
    b = (cosmac->r[cosmac->p] >> 0) & 255;
    cosmac->r[cosmac->p] += 2;
  } else {
    b = cosmac->read(cosmac, cosmac->r[cosmac->p]++);
  }
  
  if (l == 1) {
    c = cosmac->q;
  }
  
  if (l == 2) {
    c = (cosmac->d == 0);
  }
  
  if (l == 3) {
    c = cosmac->df;
  }
  
  if (cosmac->n >= 8) {
    c = !c;
  }
  
  if (c) {
    cosmac->r[cosmac->p] = b | ((uint16_t)(cosmac->b) << 8);
  }
}

void cosmac_tick(cosmac_t *cosmac) {
  if (cosmac->s == s0_fetch) {
    const uint8_t b = cosmac->read(cosmac, cosmac->r[cosmac->p]++);
    
    cosmac->i = (b >> 4) & 15;
    cosmac->n = (b >> 0) & 15;
    
    cosmac->s = s1_execute_1;
    return;
  }
  
  if (cosmac->s == s1_reset) {
    cosmac->i = 0;
    cosmac->n = 0;
    
    cosmac->ie = true;
    cosmac->q = false;
    
    cosmac->s = s1_init;
    return;
  }
  
  if (cosmac->s == s1_init) {
    cosmac->p = 0;
    cosmac->x = 0;
    
    cosmac->r[0] = 0;
    
    cosmac->s = s0_fetch;
    return;
  }
  
  if (cosmac->s == s1_execute_1) {
    tick_s1(cosmac);
    
    if (cosmac->i == 12) {
      cosmac->s = s1_execute_2;
      return;
    }
  }
  
  if (cosmac->s == s1_execute_2) {
    tick_s2(cosmac);
  }
  
  if (cosmac->s == s2_dma) {
    cosmac->r[0]++;
  }
  
  if (cosmac->s == s3_int) {
    cosmac->t = cosmac->p | (cosmac->x << 4);
    
    cosmac->p = 1;
    cosmac->x = 2;
    
    cosmac->ie = false;
  }
  
  if (cosmac->dma_line) {
    cosmac->s = s2_dma;
  } else if (cosmac->ie && cosmac->irq_line) {
    cosmac->s = s3_int;
  } else if (cosmac->s != s1_wait) {
    cosmac->s = s0_fetch;
  }
}
