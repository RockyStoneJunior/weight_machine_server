#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

#define RECVBUFF_MAX_SIZE 1024
#define PORT 9999

int main(int argc, char **argv)
{
	char recvbuff[RECVBUFF_MAX_SIZE];

	struct sockaddr_in servaddr;
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	int serverfd = socket(AF_INET, SOCK_STREAM, 0);
	bind(serverfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	listen(serverfd, 5);

	int servaddr_len = sizeof(servaddr);
	while(1)
	{
		int clientfd = accept(serverfd, (struct sockaddr *)&servaddr, &servaddr_len);
		
		int recv_len;
		while((recv_len = read(clientfd, recvbuff, RECVBUFF_MAX_SIZE)) > 0)
		{
			write(STDOUT_FILENO, recvbuff, recv_len);
			write(clientfd, recvbuff, recv_len);
		}

		close(clientfd);
	}

	return 0;
}
