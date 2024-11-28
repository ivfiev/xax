#include <string.h>
#include "scan.h"
#include "util.h"

union word64 parse_value(char *val_str, int type) {
  char *end = NULL;
  union word64 ret;
  if (type == INFER_TYPE) {
    if (strstr(val_str, ".") != NULL) {
      ret.float32 = (float)strtod(val_str, &end);
    } else {
      ret.int32 = (int)strtol(val_str, &end, 10);
    }
  } else {
    if (type == FLOAT32_TYPE) {
      ret.float32 = (float)strtod(val_str, &end);
    }
  }
  if (end == NULL || *end != 0) {
    err_fatal("parse_value");
  }
  return ret;
}

uintptr_t parse_addr(char *addr_str) {
  char *end = NULL;
  uintptr_t addr = strtoull(addr_str, &end, 16);
  if (*end != 0) {
    err_fatal("parse_addr");
  }
  if (addr % 4 != 0) {
    err_fatal("parse_addr % 4");
  }
  return addr;
}

int is_int32(union word64 word) {
  return IN_RANGE(-10000000, word.int32, 10000000);
}

int is_float32(union word64 word) {
  return IN_RANGE(-1000000.0, word.float32, 1000000.0);
}

int is_ptr32(union word64 ptr, mem_desc ds[], size_t ds_size) {
  for (int i = 0; i < ds_size; i++) {
    if (IN_RANGE(ds[i].start, ptr.ptr32, ds[i].start + ds[i].size)) {
      return 1;
    }
  }
  return 0;
}

int is_ptr64(union word64 ptr, mem_desc ds[], size_t ds_size) {
  for (int i = 0; i < ds_size; i++) {
    if (IN_RANGE(ds[i].start, ptr.ptr64, ds[i].start + ds[i].size)) {
      return 1;
    }
  }
  return 0;
}
