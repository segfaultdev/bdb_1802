#include <stdbool.h>
#include <cosmac.h>
#include <stdint.h>
#include <stdio.h>

static void bdb_write(cosmac_t *, uint16_t, uint8_t) {
  
}

static uint8_t bdb_read(cosmac_t *cosmac, uint16_t address) {
  return (rand() % 255) + 1;
}

static void bdb_write_io(cosmac_t *, uint8_t, uint8_t) {
  
}

static uint8_t bdb_read_io(cosmac_t *, uint8_t) {
  return 0;
}

cosmac_t bdb_cosmac = (cosmac_t) {
  .write = bdb_write,
  .read = bdb_read,
  
  .write_io = bdb_write_io,
  .read_io = bdb_read_io,
};

int main(void) {
  cosmac_init(&bdb_cosmac);
  
  while (true) {
    printf("(P=%02d, X=%02d)", bdb_cosmac.p, bdb_cosmac.x);
    
    for (int i = 0; i < 16; i++) {
      printf(" 0x%04X", bdb_cosmac.r[i]);
    }
    
    putchar('\n');
    cosmac_tick(&bdb_cosmac);
  }
  
  return 0;
}
