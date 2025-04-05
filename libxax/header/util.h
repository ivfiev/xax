#ifndef XAX_UTIL_H
#define XAX_UTIL_H

#include <time.h>
#include <unistd.h>
#include <stdint.h>

#define SIZEARR(arr) (sizeof(arr) / sizeof((arr)[0]))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define ABS(x) ((x) < 0 ? (-(x)) : (x))
#define FLOAT_EQ(x, y) (ABS((x) - (y)) < 1e-6)
#define FLOAT_DIFF(x, y, z) (ABS((x) - (y)) >= z)
#define IN_RANGE(a, x, b) ((a) <= (x) && (x) <= (b))

#define ERROR_FATAL(msg) err_fatal("%s at %s:%d", msg, __FILE_NAME__, __LINE__)

void err_fatal(char *);

ssize_t read_all(int fd, char buf[], size_t size);

ssize_t write_all(int fd, char buf[], size_t size);

size_t run_cmd(char *cmd, char buf[], size_t size);

size_t strsplit(char *str, const char *sep, char **toks, size_t size);

void trim_end(char *str);

int is_div_by(double x, double y);

int matches(char *ptr, int *pattern, int len);

float dist(float x0, float y0, float x1, float y1);

time_t timestamp(void);

int intcmp(const void *, const void *);

void msleep(int ms);

void disable_stderr(void);

void print_bytes(uint8_t bytes, size_t count);

#endif
