#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

int main(int argc, char **argv) {
  char *arr[MAXARG];
  int cnt = 0;
  char c;
  char buf[1000];
  int idx=0;
  while (read(0, &c, 1) != 0) {
    if (c != '\n')
      buf[idx++] = c;
    else {
      buf[idx] = 0;
      char *str = (char*)malloc(idx+1);
      strcpy(str, buf);
      arr[cnt++] = str;
      idx = 0;
    }
  }
  for (int i=0; i<cnt; ++i) {
    int pid = fork();
    if (pid == 0) {
      // child
      char *new_argv[argc];
      for (int i=0; i<argc-1; ++i) {
        new_argv[i] = argv[i+1];
      }
      new_argv[argc-1] = arr[i];
      new_argv[argc] = 0;
      exec(argv[1], new_argv);
    } else {
      // parent
      wait(0);
    }
  }
  exit(0);
}
