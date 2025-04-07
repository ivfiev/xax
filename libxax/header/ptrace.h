#ifndef H_PTRACE
#define H_PTRACE

#include <sys/user.h>
#include <sys/ptrace.h>
#include "types.h"

ssize_t ptrace_read(pid_t pid, uintptr_t addr, uint8_t buf[], size_t count);

ssize_t ptrace_write(pid_t pid, uintptr_t addr, uint8_t buf[], size_t count);

void set_byte(pid_t pid, uintptr_t addr, uint8_t byte);

void ret(pid_t tid, struct user_regs_struct *regs);

#endif