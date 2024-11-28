#ifndef XAX_MEM_H
#define XAX_MEM_H

#include <stdint.h>
#include "types.h"

#define MAX_FILES 16
#define MAX_LINES 4096
#define SAMPLE_RADIUS 2048
#define SAMPLE_SIZE (2 * SAMPLE_RADIUS + 1)
#define BYTE_STR_LEN (3 * SAMPLE_SIZE + 2)

#define PRECISION 0.01

void ptr_bfs(pid_t pid, int mem_fd, hashtable *tbl, char *lib_name, size_t lib_size, int depth, char *filename);

#endif
