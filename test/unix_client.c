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

#define MSG "hello server!"

int
main(int argc, char *argv[])
{
        int sock_fd, ret, peer_fd;
        struct sockaddr_un addr;
        char response[128];

        sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock_fd < 0) {
                perror("AF_VSOCK socket");
                return -1;
        }

        memset(&addr, 0, sizeof(struct sockaddr));

        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, TMPFILE_PATH, sizeof(addr.sun_path) - 1);

        ret = connect(sock_fd, (struct sockaddr *) &addr,
                        sizeof(struct sockaddr_un));
        if (ret < 0) {
                perror("socket connect");
                return -1;
        }

        send(sock_fd, (char *) MSG, strlen(MSG) + 1, 0);
        recv(sock_fd, (char *) response, strlen(MSG) + 1, 0);
        close(sock_fd);

        printf("%s\n", response);

        return 0;
}
