#ifndef XAX_SCAN_H
#define XAX_SCAN_H

#include "types.h"

#define SCAN(block, filter) \
  do {                                                                                                                \
    mem_block *blk = (block);                                                                                         \
    char *memory = blk->bytes;                                                                                            \
    size_t memory_size = blk->size;                                                                                            \
    uintptr_t base_addr = blk->base_addr;                                                                             \
    if (memory_size == 0) { \
      break;                                \
    }                        \
    for (uintptr_t offset = 0; offset < memory_size - 7; offset += 4) {                                                           \
      union word64 word = {.int64 = 0};                                                                               \
      memcpy(word.bytes, memory + offset, 8); \
      filter                                                                                                          \
    }                                                                                                                 \
  } while (0)                                                                                                         \

#define FOREACH_BLOCK(start_ix, end_ix, code) \
  do {                                        \
  mem_desc ds[end_ix + 1]; \
  size_t ds_size = read_mem_desc(pid, ds, SIZEARR(ds)); \
  for (int i = start_ix; i <= end_ix && i < ds_size; i++) { \
    mem_desc desc = ds[i];                    \
    mem_block *block = read_mem_block(fd, desc.start, desc.size); \
    code \
    free_mem(block); \
  }                         \
  } while (0)               \

#define WORD_ADDR (base_addr + offset)

#define INFER_TYPE 0
#define FLOAT32_TYPE 1

union word64 parse_value(char *val, int type);

uintptr_t parse_addr(char *val);

int is_int32(union word64 word);

int is_float32(union word64 word);

int is_ptr32(union word64 ptr, mem_desc ds[], size_t ds_size);

int is_ptr64(union word64 ptr, mem_desc ds[], size_t ds_size);

#endif
