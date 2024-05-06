.load 0x0000

# r0 - RP (DMA pointer, unused)
# r1 - TP (trap pointer)
# r2 - SP (stack pointer)
# r3 - IP (instruction pointer)

boot:
  

test:
  inc rf
  ret
