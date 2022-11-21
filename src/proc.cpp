#include <sys/mount.h>
#include <cstdio>

int main(int argc, char *argv[]) {
    // Remount procfs
    mount("proc", "/proc", "proc", 0, NULL);
}