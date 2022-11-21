#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sched.h>
#include <unistd.h>
#include <iostream>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <string>
#include <array>
#include <memory>

// Based on https://zserge.com/posts/containers/

static char child_stack[1024 * 1024];

std::string setup_loop(std::string filename) {
    // Get a name of a free loop device.
    // Based on https://stackoverflow.com/a/478960

    char command[128] = {0,};
    std::snprintf(command, sizeof(command), "losetup -fP --show %s", filename.c_str());

    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed for losetup");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    // Trim newlines
    if (!result.empty() && result[result.length()-1] == '\n') {
        result.erase(result.length()-1);
    }

    return result;
}

int child_main(void *arg) {
    std::cout << "Started the child process, PID = " << getpid() << std::endl;
    
    // Unmount procfs
    umount2("/proc", MNT_DETACH);
    
    // Mount the filesystem
    std::string loop = setup_loop("./rootfs.img");
    std::cout << "Loop device = " << loop << std::endl;
    int mount_result = mount(loop.c_str(), "./rootfs", "ext4", 0, "");
    if (mount_result < 0) {
        std::cerr << "mount failed: " << errno << std::endl;
        return 1;
    }
    mkdir("./rootfs/oldrootfs", 0755);

    // Fuck pivot_root, it does not work
    chroot("./rootfs");
    chdir("/");

    // Detach host filesystem
    umount2("/oldrootfs", MNT_DETACH);
    rmdir("/oldrootfs");

    // Remount procfs
    mount("proc", "/proc", "proc", 0, NULL);
    
    // Run the process
    char **argv = (char**)arg;
    execvp(argv[0], argv);
    return 0;
}

int main(int argc, char *argv[]) {
    int flags = CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWIPC | CLONE_NEWNET;
    int pid = clone(child_main, child_stack + sizeof(child_stack), flags | SIGCHLD, argv + 1);
    if (pid < 0) {
        std::cerr << "clone failed: " << errno << std::endl;
        return 1;
    }
    waitpid(pid, NULL, 0);
    return 0;
}
