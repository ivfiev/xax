#include "args.h"
#include "cotw.h"
#include <stdio.h>

__attribute__((constructor))
static void init(void) {
  args_add("cotw", run);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    puts("usage: xax \%op\% \%process_name\% \%args..\%");
    return 1;
  }
  args_parse(argc - 2, argv + 2);
  args_exec(argv[1]);
  return 0;
}
