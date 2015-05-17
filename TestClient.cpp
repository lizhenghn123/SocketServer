#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static int create_socket_client(int port)
{
	int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    assert(sock > 0);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    int ret = ::connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    assert(ret == 0);

	return sock;
}


int main(int argc, char **argv)
{
    int sock = create_socket_client(8888);

    char str[1024];
    char buf[1024];
    int len;
	for(int i = 0; i < 10; i++)
    {
		len = sprintf(str, "echo: %d\n", i);
        printf("write to server[%s]\n", str);
		::write(sock, str, len);
        
        buf[0] = '\0';
		len = ::read(sock, buf, 1024);
		buf[len] = '\0';
		//fprintf(stderr, buf);
        printf("recv from server [%s]\n", buf);
		//usleep(100 * 1000);
        sleep(1);
	}

    printf("###### GAME OVER ######\n");
	return 0;
}
