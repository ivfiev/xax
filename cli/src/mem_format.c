#include <stdio.h>
#include <stdlib.h>
#include "args.h"
#include "mem.h"
#include "util.h"

static int SAMPLE_PATTERN[SAMPLE_SIZE];
static int SUB_PATTERN_LEN;
static int SUB_PATTERN[SAMPLE_SIZE];

static void readfile(char *filename) {
  char buf[BYTE_STR_LEN * 2];
  char *toks[SAMPLE_SIZE];
  FILE *fs = fopen(filename, "r");
  fgets(buf, SIZEARR(buf), fs);
  strsplit(buf, " ", toks, SAMPLE_SIZE);
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    SAMPLE_PATTERN[i] = strtol(toks[i], NULL, 10);
  }
}

static void readsubpattern(char *subpattern) {
  char *toks[SAMPLE_SIZE];
  size_t size = strsplit(subpattern, " ", toks, SAMPLE_SIZE);
  for (int i = 0; i < size; i++) {
    SUB_PATTERN[i] = strtol(toks[i], NULL, 10);
    SUB_PATTERN_LEN++;
  }
}

static void format(void) {
  char *filename = args_get("arg0");
  char *pattern = args_get("arg1");
  readfile(filename);
  readsubpattern(pattern);
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    int j = 0;
    while (j < SUB_PATTERN_LEN && SAMPLE_PATTERN[i + j] == SUB_PATTERN[j]) {
      j++;
    }
    if (j == SUB_PATTERN_LEN) {
      int offset = SAMPLE_RADIUS - i;
      printf("Offset: [%d]\n", offset);
      for (int k = 0; k < SUB_PATTERN_LEN; k++) {
        printf("%d,", SUB_PATTERN[k]);
      }
      puts("");
    }
  }
}

__attribute__((constructor))
static void init(void) {
  args_add("format", format);
}