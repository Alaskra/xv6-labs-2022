#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p[2];
  if (pipe(p) != 0) {
    fprintf(2, "pingpong: failed to create pipe\n");
    exit(1);
  }
  char buf;

  int pid = fork();
  if (pid == 0) {
    // child
    read(p[0], &buf, 1);
    fprintf(2, "%d: received ping\n", getpid());
    write(p[1], &buf, 1);
  } else {
    // parent
    write(p[1], "0", 1);
    read(p[0], &buf, 1);
    fprintf(2, "%d: received pong\n", getpid());
  }
  close(p[0]);
  close(p[1]);

  exit(0);
}

