#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define RECVBUFF_MAX_SIZE 1024
#define PORT 9999

#define RESPONSE_TIMESTAMP "Headt2@%d\n"

void *connection_process_thread(void *);

int main(int argc, char **argv)
{
	char recvbuff[RECVBUFF_MAX_SIZE];
	pthread_t connection_process_thread_nr;

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
		
		pthread_create(&connection_process_thread_nr, NULL, connection_process_thread, &clientfd);
	}

	return 0;
}

void *connection_process_thread(void *clientfd_param)
{
	int clientfd = *(int *)clientfd_param;

	char recvbuff[RECVBUFF_MAX_SIZE];
	char sendbuff[128];	

	int recv_len;
	int send_len;

	while((recv_len = read(clientfd, recvbuff, RECVBUFF_MAX_SIZE)) > 0)
	{

		if(!strncmp(recvbuff, "Headt", 5))
		{
			send_len = sprintf(sendbuff, RESPONSE_TIMESTAMP, (int)time(NULL));
			write(clientfd, sendbuff, send_len);
		}else{
			write(clientfd, "Unknown Command\n", 16);
		}
	}

	close(clientfd);

	return NULL;
}
