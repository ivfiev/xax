#ifndef XAX_IMPL_H
#define XAX_IMPL_H

#include <sys/types.h>

#define OPEN_MEM(proc_name) \
  pid_t pid = get_pid(proc_name); \
  if (pid <= 0) { \
    err_fatal("pid"); \
  } \
  int fd = open_mem(pid)   \

#define PARSE_RANGE() \
  char from_str[16], to_str[16]; \
  if (sscanf(range_str, "(%15[^,],%15[^)])", from_str, to_str) != 2) { \
    err_fatal("bad range input"); \
  } \
  float from = parse_value(from_str, FLOAT32_TYPE).float32; \
  float to = parse_value(to_str, FLOAT32_TYPE).float32                \

#define READ_DS(size) \
  mem_desc ds[size]; \
  size_t ds_size = read_mem_desc(pid, ds, SIZEARR(ds))

#endif
