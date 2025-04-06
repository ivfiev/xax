#include "args.h"
#include "util.h"
#include "proc.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <errno.h>

static int STOPPING = 0;

static void handle_sigint(int) {
  if (STOPPING == 1) {
    exit(0);
  }
  STOPPING = 1;
}

static int handle_stop(pid_t tid) {
  struct user_regs_struct regs;
  ptrace(PTRACE_GETREGS, tid, 0, &regs);
  if (regs.orig_rax == 1) {
    return regs.rax == -ENOSYS;
  }
  return 0;
}

void run(void) {
  if (signal(SIGINT, handle_sigint) == SIG_ERR) {
    err_fatal("signal");
  }
  pid_t pid = get_pid("demo");
  if (pid <= 0) {
    err_fatal("pid");
  }
  pid_t tids[16];
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
    ptrace(PTRACE_SETOPTIONS, tids[i], 0, PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE);
    ptrace(PTRACE_SYSCALL, tids[i], 0, 0);
  }
  int status;
  for (;;) {
    pid_t tid = waitpid(-1, &status, __WALL);
    puts("caught");
    if (STOPPING == 1) {
      ptrace(PTRACE_DETACH, tid, 0, 0);
    } else {
      ptrace(PTRACE_SYSCALL, tid, 0, 0);
    }
  }
}
