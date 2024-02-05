#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

#include <linux/vm_sockets.h>

#define LISTEN_BACKLOG 50

#define TMPFILE_PATH "/tmp/krun-unix.sock"

#define MSG "hello server!"

int
main(int argc, char *argv[])
{
        int sock_fd, ret, peer_fd;
        struct sockaddr_vm addr;
        char response[128];

        sock_fd = socket(AF_VSOCK, SOCK_STREAM, 0);
        if (sock_fd < 0) {
                perror("AF_VSOCK socket");
                return -1;
        }

        memset(&addr, 0, sizeof(struct sockaddr_vm));

        addr.svm_family = AF_VSOCK;
        addr.svm_port = 1025;
        addr.svm_cid = 3;

        ret = connect(sock_fd, &addr, sizeof(struct sockaddr_vm));
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
