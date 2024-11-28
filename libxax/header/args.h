#ifndef XAX_ARGS_H
#define XAX_ARGS_H

void args_add(char *key, void *val);

char *args_get(char *key);

void args_exec(char *key);

void args_parse(int, char **);

#endif
