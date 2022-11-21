#include <sched.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>

static char child_stack[1024 * 1024];

int child_main(void *arg) {
  printf("Hello from child! PID=%d\n", getpid());
  return 0;
}

int main(int argc, char *argv[]) {
  int flags =
      CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWIPC | CLONE_NEWNET;
  int pid = clone(child_main, child_stack + sizeof(child_stack),
                  flags | SIGCHLD, argv + 1);
  if (pid < 0) {
    fprintf(stderr, "clone failed: %d\n", errno);
    return 1;
  }
  waitpid(pid, NULL, 0);
  return 0;
}
