#ifndef XAX_TYPES_H
#define XAX_TYPES_H

#include <stdint.h>
#include <stdlib.h>

union word64 {
  char bytes[8];
  int int32;
  short int16;
  float float32;
  long long int64;
  double float64;
  uint32_t ptr32;
  uintptr_t ptr64;
};

union word32 {
  char bytes[4];
  short int16[2];
  int int32;
  float float32;
  uint32_t ptr;
};

typedef struct {
  char name[256];
  uintptr_t start;
  size_t size;
} mem_desc;

typedef struct {
  char *bytes;
  uintptr_t base_addr;
  size_t size; // bytes count, arr may be larger.
} mem_block;

typedef union keyval_t {
  void *ptr;
  char *str;
  uint64_t uint64;
  int int32;
  float float32;
} kv;

struct node {
  kv key;
  kv val;
  struct node *next;
};

typedef struct hashtable {
  struct node **nodes;
  size_t cap;
  size_t len;

  int (*cmp)(kv k1, kv k2);

  uint64_t (*hash)(kv k, uint64_t N);
} hashtable;

struct entry {
  uintptr_t addr;
  uintptr_t val;
};

#endif
