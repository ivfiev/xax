#include "args.h"
#include "util.h"
#include "proc.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ptrace.h>

void run(void) {
  pid_t pid = get_pid("CotW");
  if (pid <= 0) {
    err_fatal("pid");
  }
  printf("%d\n", pid);
  if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) < 0) {
    err_fatal("attach");
  }
  waitpid(pid, NULL, 0);
  if (ptrace(PTRACE_DETACH, pid, NULL, NULL) < 0) {
    err_fatal("detach");
  }
  puts("all done!");
}
