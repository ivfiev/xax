#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "proc.h"
#include "util.h"
#include <sys/ptrace.h>

pid_t get_pid(const char *proc_name) {
  char buf[32], cmd[32];
  snprintf(cmd, SIZEARR(cmd), "pgrep %s", proc_name);
  run_cmd(cmd, buf, SIZEARR(buf));
  pid_t pid = (pid_t)strtol(buf, NULL, 10);
  return pid;
}

size_t read_mem_desc(pid_t pid, mem_desc ds[], size_t size) {
  char buf[256 * size];
  char cmd[128];
  char *lines[size];
  snprintf(cmd, SIZEARR(cmd), "cat /proc/%d/maps | grep -v render | awk '{print $1 \"|\" $6,$7,$8,$9}'", pid);
  run_cmd(cmd, buf, 256 * size);
  size_t count = strsplit(buf, "\n", lines, SIZEARR(lines));
  int i;
  for (i = 0; i < MIN(count, size); i++) {
    char *words[] = {NULL, NULL};
    char *hexes[] = {NULL, NULL};
    strsplit(lines[i], "|", words, SIZEARR(words));
    strsplit(words[0], "-", hexes, SIZEARR(hexes));
    if (words[1] != NULL) {
      strncpy(ds[i].name, words[1], SIZEARR(ds->name));
    } else {
      strncpy(ds[i].name, "NULL", SIZEARR(ds->name));
    }
    uintptr_t start = strtoull(hexes[0], NULL, 16);
    uintptr_t end = strtoull(hexes[1], NULL, 16);
    ds[i].start = start;
    ds[i].size = end - start;
  }
  return i;
}

size_t read_mem_blocks(pid_t pid, int mem_fd, mem_block *bs[], size_t size) {
  mem_desc ds[size];
  size_t ds_count = read_mem_desc(pid, ds, size);
  for (int i = 0; i < ds_count; i++) {
    bs[i] = read_mem_block(mem_fd, ds[i].start, ds[i].size);
  }
  return ds_count;
}

int find_mem_desc(const char *key, mem_desc ds[], size_t size) {
  for (int i = 0; i < size; i++) {
    if (strcasestr(ds[i].name, key)) {
      return i;
    }
  }
  return -1;
}

uintptr_t get_base_addr(pid_t pid, const char *key) {
  mem_desc ds[2048];
  size_t ds_size = read_mem_desc(pid, ds, SIZEARR(ds));
  int desc = find_mem_desc(key, ds, ds_size);
  if (0 <= desc) {
    return ds[desc].start;
  }
  return 0;
}

int open_mem(pid_t pid) {
  char path[32];
  snprintf(path, SIZEARR(path), "/proc/%d/mem", pid);
  int fd = open(path, O_RDWR);
  return fd;
}

mem_block *read_mem_block(int fd, uintptr_t addr, size_t size) {
  mem_block *mem = malloc(sizeof(mem_block));
  mem->bytes = calloc(size, sizeof(char));
  lseek(fd, (off_t)addr, SEEK_SET);
  ssize_t count = read_all(fd, mem->bytes, size);
  if (count <= 0) {
    fprintf(stderr, "failed to read block 0x%lx\n", addr);
    mem->base_addr = addr;
    mem->size = 0;
  } else {
    mem->base_addr = addr;
    mem->size = count;
  }
  return mem;
}

ssize_t read_mem_bytes(int fd, uintptr_t addr, char buf[], size_t size) {
  lseek(fd, (off_t)addr, SEEK_SET);
  return read_all(fd, buf, size);
}

union word32 read_mem_word32(int mem_fd, uintptr_t addr) {
  union word32 word = {.int32 = 0};
  read_mem_bytes(mem_fd, addr, word.bytes, 4);
  return word;
}

union word64 read_mem_word64(int mem_fd, uintptr_t addr) {
  union word64 word = {.int64 = 0};
  read_mem_bytes(mem_fd, addr, word.bytes, 8);
  return word;
}

uintptr_t hop(int mem_fd, uintptr_t base, uintptr_t offsets[], size_t size) {
  for (int i = 0; i < size - 1; i++) {
    base = read_mem_word64(mem_fd, base + offsets[i]).ptr64;
  }
  return base + offsets[size - 1];
}

size_t write_mem(int fd, uintptr_t addr, char buf[], size_t size) {
  lseek(fd, (off_t)addr, SEEK_SET);
  return write(fd, buf, size);
}

int close_mem(int fd) {
  return close(fd);
}

void free_mem(mem_block *mem) {
  free(mem->bytes);
  free(mem);
}

ssize_t ptrace_read(pid_t pid, void *addr, uint8_t buf[], size_t count) {
  #define WORD_SIZE 8
  if (count % WORD_SIZE != 0) {
    err_fatal("count %% WORD_SIZE");
  }
  size_t i;
  union word64 word;
  for (i = 0; i < count; i += WORD_SIZE) {
    word.int64 = ptrace(PTRACE_PEEKDATA, pid, addr + i, 0);
    memcpy(buf + i, word.bytes, WORD_SIZE);
  }
  return i >= count ? i : -1;
}

ssize_t ptrace_write(pid_t pid, void *addr, uint8_t buf[], size_t count) {
  #define WORD_SIZE 8
  if (count % WORD_SIZE != 0) {
    err_fatal("cound %% WORD_SIZE");
  }
  size_t i;
  union word64 word;
  for (i = 0; i < count; i += WORD_SIZE) {
    memcpy(word.bytes, buf + i, WORD_SIZE);
    ptrace(PTRACE_POKEDATA, pid, addr + i, word.int64);
  }
  return i >= count ? i : -1;
}

size_t read_tids(pid_t pid, pid_t tids[], size_t size) {
  char buf[4096];
  char *toks[256];
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "ls /proc/%d/task", pid);
  ssize_t bytes = run_cmd(cmd, buf, sizeof(buf));
  if (bytes <= 0) {
    err_fatal("read_tids");
  }
  size_t count = strsplit(buf, "\n", toks, SIZEARR(toks));
  for (int i = 0; i < count; i++) {
    tids[i] = atoi(toks[i]);
  }
  return count;
}
