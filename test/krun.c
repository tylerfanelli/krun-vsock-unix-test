/*
 * This is an example implementing chroot-like functionality with libkrun.
 *
 * It executes the requested command (relative to NEWROOT) inside a fresh
 * Virtual Machine created and managed by libkrun.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <libkrun.h>
#include <getopt.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_ARGS_LEN 4096
#ifndef MAX_PATH
#define MAX_PATH 4096
#endif

#define TMPFILE_PATH "/tmp/krun-unix.sock"

int
main(int argc, char *const argv[])
{
    char *const envp[] =
    {
        "TEST=works",
        0
    };
    char *const port_map[] =
    {
        "18000:8000",
        0
    };
    char *const rlimits[] =
    {
        // RLIMIT_NPROC = 6
        "6=4096:8192",
        0
    };
    char *mapped_volumes[2];
    char current_path[MAX_PATH];
    char volume_tail[] = ":/work\0";
    char *volume;
    int volume_len;
    int ctx_id;
    int err;
    int i;

    err = krun_set_log_level(0);
    if (err) {
        errno = -err;
        perror("Error configuring log level");
        return -1;
    }

    ctx_id = krun_create_ctx();
    if (ctx_id < 0) {
        errno = -ctx_id;
        perror("Error creating configuration context");
        return -1;
    }

    if (err = krun_set_vm_config(ctx_id, 1, 512)) {
        errno = -err;
        perror("Error configuring the number of vCPUs and/or the amount of RAM");
        return -1;
    }

    if (err = krun_set_root(ctx_id, "./fedora-test")) {
        errno = -err;
        perror("Error configuring root path");
        return -1;
    }

    if (getcwd(&current_path[0], MAX_PATH) == NULL) {
        errno = -err;
        perror("Error getting current directory");
        return -1;
    }

    volume_len = strlen(current_path) + strlen(volume_tail) + 1;
    volume = malloc(volume_len);
    if (volume == NULL) {
        errno = -err;
        perror("Error allocating memory for volume string");
    }

    snprintf(volume, volume_len, "%s%s", current_path, volume_tail);
    mapped_volumes[0] = volume;
    mapped_volumes[1] = 0;

    // Map "/tmp" as "/work" inside the VM.
    if (err = krun_set_mapped_volumes(ctx_id, &mapped_volumes[0])) {
        errno = -err;
        perror("Error configuring mapped volumes");
        return -1;
    }

    if (err = krun_set_port_map(ctx_id, &port_map[0])) {
        errno = -err;
        perror("Error configuring port map");
        return -1;
    }

    // Configure the rlimits that will be set in the guest
    if (err = krun_set_rlimits(ctx_id, &rlimits[0])) {
        errno = -err;
        perror("Error configuring rlimits");
        return -1;
    }

    // Set the working directory to "/", just for the sake of completeness.
    if (err = krun_set_workdir(ctx_id, "/")) {
        errno = -err;
        perror("Error configuring \"/\" as working directory");
        return -1;
    }

    // Specify the path of the binary to be executed in the isolated context, relative to the root path.
    if (err = krun_set_exec(ctx_id, "/bin/krun-sock-cli-test", NULL, &envp[0])) {
        errno = -err;
        perror("Error configuring the parameters for the executable to be run");
        return -1;
    }

    if (err = krun_add_vsock_port(ctx_id, 1025, TMPFILE_PATH)) {
        errno = -err;
        perror("Error adding vsock port");
        return -1;
    }

    // Start and enter the microVM. Unless there is some error while creating the microVM
    // this function never returns.
    if (err = krun_start_enter(ctx_id)) {
        errno = -err;
        perror("Error creating the microVM");
        return -1;
    }

    // Not reached.
    return 0;
}
