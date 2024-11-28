#include <stdio.h>
#include <string.h>
#include "args.h"
#include "util.h"
#include "mem.h"
#include "hashtable.h"

#define ZIP(arr1, arr2, new_arr_name, score_name) \
  int new_arr_name[SAMPLE_SIZE];                  \
  int score_name = 0;                                            \
  do {                              \
    for (int i = 0; i < SAMPLE_SIZE; i++) { \
      if (arr1[i] == arr2[i] && arr1[i] > 0) { \
        new_arr_name[i] = arr1[i];                \
        score_name++;                                      \
      } else {                                \
        new_arr_name[i] = -1;                                        \
      }\
    }\
  } while (0)                                  \

char *FILENAMES[MAX_FILES];
hashtable *BYTES[MAX_FILES];

static int filenames(void) {
  char key[8];
  int i;
  for (i = 0; i < SIZEARR(FILENAMES); i++) {
    snprintf(key, 8, "arg%d", i);
    char *filename = args_get(key);
    if (!strcmp(filename, "")) {
      break;
    }
    FILENAMES[i] = filename;
  }
  return i;
}

void readfiles(void) {
  char line_buf[32 * 1024];
  char *byte_toks[SAMPLE_SIZE];
  for (int file = 0; FILENAMES[file]; file++) {
    BYTES[file] = hash_new(MAX_LINES, hash_int, hash_cmp_int);
    FILE *fs = fopen(FILENAMES[file], "r");
    for (int line = 0; fgets(line_buf, SIZEARR(line_buf), fs); line++) {
      size_t size = strsplit(line_buf, " ", byte_toks, SIZEARR(byte_toks));
      int *bytes = calloc(size, sizeof(int));
      hash_set(BYTES[file], KV(.int32 = line), KV(.ptr = bytes));
      for (int i = 0; i < size; i++) {
        int byte = (int)strtol(byte_toks[i], NULL, 16);
        bytes[i] = byte;
      }
    }
  }
}

int *get_row(int file_ix, int line_ix) {
  return (int *)hash_getv(BYTES[file_ix], KV(.int32 = line_ix)).ptr;
}

static int MAX_SCORE;
static int MAX_PATTERN[SAMPLE_SIZE];

void max_pattern(int file_ix, int *curr_pattern, int curr_score) {
  if (!BYTES[file_ix]) {
    MAX_SCORE = curr_score;
    memcpy(MAX_PATTERN, curr_pattern, sizeof(MAX_PATTERN));
    return;
  }
  for (int line_ix = 0; line_ix < BYTES[file_ix]->len; line_ix++) {
    int *row = get_row(file_ix, line_ix);
    if (curr_pattern == NULL) {
      max_pattern(file_ix + 1, row, -1);
      continue;
    }
    ZIP(curr_pattern, row, new_pattern, new_score);
    if (new_score > MAX_SCORE) {
      max_pattern(file_ix + 1, new_pattern, new_score);
    }
  }
}

static void sigscan(void) {
  filenames();
  readfiles();
  max_pattern(0, NULL, -1);
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    printf("%d ", MAX_PATTERN[i]);
  }
  puts("");
  fprintf(stderr, "%d\n", MAX_SCORE); // meh
}

__attribute__((constructor))
static void init(void) {
  args_add("sigscan", sigscan);
}
