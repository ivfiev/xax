#include <unistd.h>
#include "util.h"
#include "types.h"
#include "scan.h"
#include "proc.h"
#include "hashtable.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#define MIN_PTR 0x500000000000
#define MAX_PTR 0x800000000000

static size_t entries_filter(pid_t pid, int fd, struct entry *entries, size_t size) {
  size_t entry_ix = 0;
  FOREACH_BLOCK(1, 2000, {
    SCAN(block, {
      if (entry_ix >= size) {
        perror("need moar MEMORY");
        exit(1);
      }
      if (IN_RANGE(MIN_PTR, word.ptr64, MAX_PTR)) {
        entries[entry_ix].addr = WORD_ADDR;
        entries[entry_ix++].val = word.ptr64;
      }
    });
  });
  return entry_ix;
}

static int cmp_entries(const void *v1, const void *v2) {
  struct entry *e1 = (struct entry *)v1;
  struct entry *e2 = (struct entry *)v2;
  if (e1->val < e2->val) {
    return -1;
  } else if (e1->val > e2->val) {
    return 1;
  } else {
    return 0;
  }
}

static void entries_sort(struct entry *entries, size_t size) {
  qsort(entries, size, sizeof(struct entry), cmp_entries);
}

static size_t entries_bsearch(uintptr_t addr, struct entry *entries, size_t size) {
  size_t i = 0, j = size;
  while (j - i > 2) {
    int mid = (i + j) / 2;
    if (entries[mid].val < addr) {
      i = mid;
    } else {
      j = mid + 1;
    }
  }
  while (i < size && entries[i].val < addr) {
    i++;
  }
  return i;
}

static char CURRENT_PATH[48];
static hashtable *RESULTS;

static void reset_results(void) {
  if (RESULTS != NULL) {
    FOREACH_KV(RESULTS, {
      free(key.ptr);
    });
    hash_free(RESULTS);
  }
  RESULTS = hash_new(1000000, hash_str, hash_cmp_str);
}

static void dump_ptr_paths(int mem_fd, hashtable *paths, uintptr_t ptr, int p_ix, int depth) {
  if (depth < 0 || p_ix >= SIZEARR(CURRENT_PATH)) {
    return;
  }
  hashtable *adj = hash_getv(paths, KV(.uint64 = ptr)).ptr;
  if (adj == NULL) {
    if (!hash_hask(RESULTS, KV(.ptr = CURRENT_PATH))) {
      char *copy = strdup(CURRENT_PATH);
      hash_set(RESULTS, KV(.ptr = copy), KV(.uint64 = 1));
    }
    return;
  }
  uintptr_t entry_val = read_mem_word64(mem_fd, ptr).ptr64;
  FOREACH_KV(adj, {
    uintptr_t field_offset = key.uint64;
    uintptr_t next = entry_val + field_offset;
    int p_ix_new = p_ix + snprintf(CURRENT_PATH + p_ix, SIZEARR(CURRENT_PATH) - p_ix, ", 0x%lx", field_offset);
    dump_ptr_paths(mem_fd, paths, next, p_ix_new, depth - 1);
  });
}

void ptr_bfs(pid_t pid, int mem_fd, hashtable *tbl, char *lib_name, size_t lib_size, int depth, char *filename) {
  reset_results();
  int file_fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
  const int ptr_radius = 2048;
  const size_t ENTRIES_SIZE = 100000000;
  struct entry *es = calloc(ENTRIES_SIZE, sizeof(struct entry));
  size_t es_count = entries_filter(pid, mem_fd, es, ENTRIES_SIZE);
  printf("#Entries: [%ld]\n", es_count);
  entries_sort(es, es_count);
  size_t lib_start = get_base_addr(pid, lib_name);
  hashtable *paths = hash_new(ptr_radius * ptr_radius, hash_int, hash_cmp_int);
  hashtable *done = hash_new(ptr_radius * ptr_radius, hash_int, hash_cmp_int);
  for (int d = 0; d < depth; d++) {
    printf("Scanning depth [%d], #addrs [%ld], static range: [0x%lx-0x%lx]\n", d + 1, tbl->len, lib_start, lib_start + lib_size);
    FOREACH_KV(tbl, {
      uintptr_t val_addr = key.uint64;
      size_t ei = entries_bsearch(val_addr - ptr_radius, es, es_count);
      for (; es[ei].val <= val_addr; ei++) {
        uintptr_t field_offset = val_addr - es[ei].val; // offset from beginning to value in question
        uintptr_t entry_addr = es[ei].addr; // addr of the ptr
        uintptr_t entry_val = es[ei].val; // ptr to beginning of the struct
        if (hash_hask(done, KV(.uint64 = entry_addr))) {
          // avoid cycles
          continue;
        }
        hashtable *adj = hash_getv(paths, KV(.uint64 = entry_addr)).ptr; // all structs entry_val potentially points to
        if (adj == NULL) {
          adj = hash_new(32, hash_int, hash_cmp_int);
          hash_set(paths, KV(.uint64 = entry_addr), KV(.ptr = adj));
        }
        hash_set(adj, KV(.uint64 = field_offset), KV(.uint64 = 1));
        hash_set(tbl, KV(.uint64 = entry_addr), KV(.uint64 = entry_val));
        if (IN_RANGE(lib_start, entry_addr, lib_start + lib_size)) {
          dump_ptr_paths(mem_fd, paths, entry_addr,
            snprintf(CURRENT_PATH, SIZEARR(CURRENT_PATH), "0x%lx", entry_addr - lib_start), depth);
        }
      }
      hash_del(tbl, key);
      hash_set(done, key, KV(.uint64 = 1));
    });
  }
  FOREACH_KV(RESULTS, {
    size_t len = strlen(key.ptr);
    if (write_all(file_fd, key.ptr, len) < len || write(file_fd, "\n", 1) <= 0) {
      fprintf(stderr, "error writing [%s]\n", strerror(errno));
    }
  });
  FOREACH_KV(paths, {
    hash_free(val.ptr);
  });
  hash_free(paths);
  free(es);
  close(file_fd);
}