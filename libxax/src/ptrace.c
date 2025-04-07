#include <string.h>
#include "util.h"
#include "types.h"
#include <sys/ptrace.h>
#include <sys/user.h>

ssize_t ptrace_read(pid_t pid, uintptr_t addr, uint8_t buf[], size_t count) {
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

ssize_t ptrace_write(pid_t pid, uintptr_t addr, uint8_t buf[], size_t count) {
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

void set_byte(pid_t pid, uintptr_t addr, uint8_t byte) {
  uint8_t bytes[8];
  ptrace_read(pid, addr, bytes, SIZEARR(bytes));
  bytes[0] = byte;
  ptrace_write(pid, addr, bytes, SIZEARR(bytes));
}

void ret(pid_t tid, struct user_regs_struct *regs) {
  union word64 w;
  ptrace_read(tid, regs->rsp, (uint8_t *)w.bytes, sizeof(w.bytes));
  regs->rip = w.int64;
  regs->rsp += 8;
}