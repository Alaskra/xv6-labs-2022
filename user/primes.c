#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void recu(int p[2]) {
  close(p[1]);
  int prime;
  if (read(p[0], &prime, 4) != 0) {
    int pread = p[0];
    if (pipe(p) != 0) {
      fprintf(2, "primes: failed to create pipe\n");
      exit(1);
    }
    fprintf(2, "prime %d\n", prime);
    int pid = fork();
    if (pid == 0) {
      // child
      recu(p);
    } else {
      // parent
      int num;
      while (read(pread, &num, 4) != 0) {
        if (num % prime != 0) {
          write(p[1], &num, 4);
        }
      }
      close(pread);
      close(p[0]);
      close(p[1]);
      wait(0);
    }
  }
  exit(0);
}

int main(int argc, char *argv[]) {
  int p[2];
  if (pipe(p) != 0) {
    fprintf(2, "primes: failed to create pipe\n");
    exit(1);
  }
  int pid = fork();
  if (pid == 0) {
    // child
    recu(p);
  } else {
    // parent
    int num=2;
    for (; num <= 35; ++num) {
        write(p[1], &num, 4);
    }
    close(p[0]);
    close(p[1]);
    wait(0);
  }
  exit(0);
}


