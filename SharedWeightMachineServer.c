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

#define RESPONSE_TIMESTAMP "Heart@%d\n"

#define WX_PLATFORM_LINK "Transmission Links@http://weixin.qq.com/q/02ta7UkQ4de921tsBFNr1g\n"

void *connection_process_thread(void *);
unsigned char calcFCS(unsigned char *pMsg, unsigned char len);

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

	pid_t tid = syscall(SYS_gettid);

	while((recv_len = read(clientfd, recvbuff, RECVBUFF_MAX_SIZE)) > 0)
	{
		time_t current_time = time(NULL);
		char *c_time_string = ctime(&current_time);

		if(!strncmp(recvbuff, "Heart", 5))
		{
			send_len = sprintf(sendbuff, RESPONSE_TIMESTAMP, current_time);
			write(clientfd, sendbuff, send_len);

			printf("%s---TID:%d Heart---\n", c_time_string,tid);
		}else{
			unsigned char checksum = calcFCS(recvbuff, 10);
		
			if((recvbuff[0] == 0x5A) && (recvbuff[1] == 0x0B) && (recvbuff[10] == checksum))
			{
				int wx_platform_link_len = strlen(WX_PLATFORM_LINK);	
				write(clientfd, WX_PLATFORM_LINK, wx_platform_link_len);

				printf("%s---TID:%d machineid[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] weight[0x%x 0x%x]---\n", c_time_string, tid, recvbuff[2], recvbuff[3], recvbuff[4], recvbuff[5], recvbuff[6], recvbuff[7], recvbuff[8], recvbuff[9]);
			}else if(!strncmp(recvbuff, "Links Success", 13)){
				printf("%s---TID:%d Links Success---\n", c_time_string, tid);
			}else{
				write(clientfd, "Unknown Command\n", 16);
				printf("%s---TID:%d Unknown Command---\n", c_time_string, tid);
			}
		}
	}

	close(clientfd);

	return NULL;
}

unsigned char calcFCS(unsigned char *pMsg, unsigned char len)
{
	unsigned char result = 0;

	while(len--)
	{
		result ^= *pMsg++;
	}

	return result;
}
