/*
 * tcp_echo.c - minimal TCP echo server for bring-up
 * Usage: tcp_echo [port=5000]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char **argv)
{
	int port = (argc >= 2) ? atoi(argv[1]) : 5000;
	int sfd, cfd;
	struct sockaddr_in addr;
	char buf[256];
	ssize_t n;

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd < 0) {
		perror("socket");
		return 1;
	}

	int yes = 1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	}
	if (listen(sfd, 1) < 0) {
		perror("listen");
		return 1;
	}

	printf("tcp_echo listening on %d\n", port);
	fflush(stdout);

	for (;;) {
		cfd = accept(sfd, NULL, NULL);
		if (cfd < 0) {
			perror("accept");
			continue;
		}
		while ((n = read(cfd, buf, sizeof(buf))) > 0) {
			if (write(cfd, buf, n) != n)
				break;
		}
		close(cfd);
	}
	return 0;
}
