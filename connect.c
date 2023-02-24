#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int main(int argc, char** argv) {
  int p[4];
  pipe(p);
  pipe(p+2);
  if (fork()) {
    dup2(p[0], 0);
    dup2(p[3], 1);
    close(p[0]);
    close(p[1]);
    close(p[2]);
    close(p[3]);
    execlp("bash", "bash", "-c", argv[1], 0);
  }
  if (fork()) {
    dup2(p[2], 0);
    dup2(p[1], 1);
    close(p[0]);
    close(p[1]);
    close(p[2]);
    close(p[3]);
    execlp("bash", "bash", "-c", argv[2], 0);
  }
  wait(0);
  wait(0);
}
