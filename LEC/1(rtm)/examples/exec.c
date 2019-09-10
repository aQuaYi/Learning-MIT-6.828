
// exec.c: replace a process with an executable file

#include "kernel/types.h"
#include "user/user.h"

int
main()
{
  char *argv[] = { "echo", "hello", 0 };

  exec("echo", argv);

  printf("exec failed!\n");

  exit();
}
