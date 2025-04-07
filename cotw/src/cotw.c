#include "args.h"
#include "util.h"
#include "proc.h"
#include "ptrace.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <errno.h>
#include <string.h>

//#define ADDRESS 0x401187
#define ADDRESS 0x140a22a78

static int DETACHING = 0;

static void handle_sigint(int) {
  if (DETACHING == 1) {
    exit(0);
  }
  DETACHING = 1;
}

static int handle_breakpoint(pid_t tid) {
  struct user_regs_struct regs;
  struct user_fpregs_struct fpregs;
  ptrace(PTRACE_GETREGS, tid, 0, &regs);
  ptrace(PTRACE_GETFPREGS, tid, 0, &fpregs);
  float x = *(float *)fpregs.xmm_space;
  //printf("%d -> %f\n", tid, x);
  x = 0.0;
  fpregs.xmm_space[0] = *(int *)(&x);
  ret(tid, &regs);
  ptrace(PTRACE_SETREGS, tid, 0, &regs);
  ptrace(PTRACE_SETFPREGS, tid, 0, &fpregs);
  ptrace(PTRACE_CONT, tid, 0, 0);
  return 0;
}

void run(void) {
  if (signal(SIGINT, handle_sigint) == SIG_ERR) {
    err_fatal("signal");
  }
  pid_t pid = get_pid("CotW");
  if (pid <= 0) {
    err_fatal("pid");
  }
  pid_t tids[128];
  size_t tids_count = read_tids(pid, tids, SIZEARR(tids));
  printf("tids count: [%ld]: ", tids_count);
  for (int i = 0; i < tids_count; i++) {
    printf("%d ", tids[i]);
  }
  puts("");
  for (int i = 0; i < tids_count; i++) {
    printf("attaching %d\n", tids[i]);
    if (ptrace(PTRACE_ATTACH, tids[i], NULL, NULL) < 0) {
      err_fatal("attach");
    }
    waitpid(tids[i], NULL, 0);
    if (i == 0) {
      set_byte(pid, ADDRESS, 0xCC);
    }
    ptrace(PTRACE_SETOPTIONS, tids[i], 0, PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE);
    ptrace(PTRACE_CONT, tids[i], 0, 0);
  }
  int status;
  for (;;) {
    pid_t tid = waitpid(-1, &status, __WALL);
    if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
      if (DETACHING == 1) {
        set_byte(tid, ADDRESS, 0xC3);
      }
      handle_breakpoint(tid);
    }
    if (DETACHING == 1) {
      ptrace(PTRACE_DETACH, tid, 0, 0);
    }
  }
}
