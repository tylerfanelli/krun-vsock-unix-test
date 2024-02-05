#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

#define LISTEN_BACKLOG 50

#define TMPFILE_PATH "/tmp/krun-unix.sock"

#define MSG "hello client!"

void cleanup();

int sock_fd = 0;

void
cleanup(int signum)
{
        int ret;

        ret = shutdown(sock_fd, SHUT_RDWR);
        if (ret < 0)
                perror("socket shutdown");

        ret = close(sock_fd);
        if (ret < 0)
                perror("socket close");

        ret = unlink(TMPFILE_PATH);
        if (ret < 0)
                perror("TMPFILE_PATH unlink");

        exit(0);
}

int
main(int argc, char *argv[])
{
        int ret, peer_fd;
        socklen_t peer_addr_size;
        struct sockaddr_un addr, peer_addr;
        char msg[128];

        signal(SIGTERM, cleanup);
        signal(SIGINT, cleanup);
        signal(SIGKILL, cleanup);

        /*
         * Create and listen on the socket.
         */
        sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock_fd < 0) {
                perror("AF_UNIX socket");
                return -1;
        }

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, TMPFILE_PATH, sizeof(addr.sun_path) - 1);

        ret = bind(sock_fd, (struct sockaddr *) &addr, sizeof(addr));
        if (ret < 0) {
                perror("socket bind");
                cleanup(0);
        }

        ret = listen(sock_fd, LISTEN_BACKLOG);
        if (ret < 0) {
                perror("socket listen");
                cleanup(0);
        }

        peer_addr_size = sizeof(peer_addr);
        peer_fd = accept(sock_fd, (struct sockaddr *) &peer_addr,
                        &peer_addr_size);
        if (peer_fd < 0) {
                perror("socket accept");
                cleanup(0);
        }

        read(peer_fd, (char *) msg, strlen(MSG) + 1);

        printf("%s\n", msg);

        write(peer_fd, (char *) MSG, strlen(MSG) + 1);

        cleanup(0);
}
