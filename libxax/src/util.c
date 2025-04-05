#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <sys/time.h>
#include <fcntl.h>
#include "types.h"

void err_fatal(char *s) {
  fprintf(stderr, "%s\n%s\n", s, strerror(errno));
  exit(1);
}

ssize_t read_all(int fd, char buf[], size_t size) {
  size_t total = 0;
  ssize_t bytes;
  do {
    bytes = read(fd, buf + total, size - total);
    if (bytes < 0) {
      fprintf(stderr, "%s\n", strerror(errno));
      return -1;
    }
    total += bytes;
  } while (bytes > 0);
  return total;
}

ssize_t write_all(int fd, char buf[], size_t size) {
  ssize_t total = 0;
  do {
    ssize_t written = write(fd, buf + total, size - total);
    if (written <= 0) {
      err_fatal("write");
    }
    total += written;
  } while (total < size);
  return total;
}

size_t run_cmd(char *cmd, char buf[], size_t size) {
  FILE *fp = popen(cmd, "r");
  if (fp == NULL) {
    err_fatal(cmd);
  }
  int fd = fileno(fp);
  ssize_t total = read_all(fd, buf, size - 1);
  if (total <= 0) {
    err_fatal(cmd);
  }
  buf[total] = 0;
  fclose(fp);
  return total;
}

size_t strsplit(char *str, const char *sep, char **toks, size_t size) {
  size_t i = 0;
  char *save_ptr;
  char *tmp = strtok_r(str, sep, &save_ptr);
  for (toks[i++] = tmp; i < size && tmp != NULL;) {
    tmp = strtok_r(NULL, sep, &save_ptr);
    if (tmp != NULL) {
      toks[i++] = tmp;
    }
  }
  return i;
}

void trim_end(char *str) {
  char *ptr = str + strlen(str) - 1;
  while (str <= ptr && isspace(*ptr)) {
    *ptr-- = '\0';
  }
}

int is_div_by(double x, double y) {
  double r = x / y;
  return fabs(r - round(r)) < 10e-12;
}

int matches(char *ptr, int *pattern, int len) {
  while (len > 0 && ((uint8_t)*ptr == *pattern) || *pattern == -1) {
    len--;
    pattern++;
    ptr++;
  }
  return len == 0;
}

float dist(float x0, float y0, float x1, float y1) {
  return sqrt(pow(x0 - x1, 2) + pow(y0 - y1, 2));
}

time_t timestamp(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000LL) + (tv.tv_usec / 1000);
}

int intcmp(const void *ptr1, const void *ptr2) {
  int i = *(int *)ptr1;
  int j = *(int *)ptr2;
  return i - j;
}

void msleep(int ms) {
  usleep(ms * 1000);
}

void disable_stderr(void) {
  int null_fd = open("/dev/null", O_WRONLY);
  if (null_fd == -1) {
    perror("Failed to open /dev/null");
    return;
  }
  if (dup2(null_fd, STDERR_FILENO) == -1) {
    perror("Failed to redirect stderr");
    close(null_fd);
  }
  close(null_fd);
}