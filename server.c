#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#define MAX 1024


typedef struct{

    enum {Request, Reply} messageType;     /* same size as an unsigned int */

    unsigned int RPCId;                    /* unique identifier */

    unsigned int procedureId;              /* e.g. (1, 2, 3, 4) for (+, -, *, /) */

    int arg1;                              /* argument/ return parameter */

    int arg2;                              /* argument/ return parameter */

    int continuation;

    int current;                            /* answer from previous expression */

    int close;

} Message;                          /* each int (and unsigned int) is 32 bits = 4 bytes */




int evaluate(int x, int y, int operator) {
	if (operator == 1)
		return (x+y);
	else if (operator == 2)
		return (x - y);
	else if (operator == 3)
		return (x * y);
	else if (operator == 4) {
		if (y == 0) {
			perror("ERROR: Divide by Zero\n");
            		return -1;
		}
		else {
			return (x / y);
		}
	}
	else {
		perror("ERROR in input formatting\n");
		exit(EXIT_FAILURE);
	}
}


int main() {
	Message *message = malloc(sizeof(Message));
	struct sockaddr_in servaddr, cliaddr;
	
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);  // socket declaration
	if (sockfd == -1) {
		perror("Failed to create socket\n");  // error handling
		exit(EXIT_FAILURE);
	}
	
	memset(&servaddr, '\0', sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(8080);
	servaddr.sin_addr.s_addr = INADDR_ANY;
	
	int rc = bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));  // socket binding
	if (rc == -1) {
		perror("Failed to bind\n");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	
	socklen_t len = sizeof(cliaddr);
	for (;;) {
		memset(message, '\0', sizeof(*message));
		ssize_t n = recvfrom(sockfd, message, sizeof(*message), MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
		
		if (n < 0) {
			perror("Failed to receive\n");
			close(sockfd);
			exit(EXIT_FAILURE);
		}
		
		if (message->close == 1)
			break;
			
		printf("procedure ID: %d\n", message->procedureId);
		
		message->current = evaluate(message->arg1, message->arg2, (int)message->procedureId);
		
		printf("Answer: %d\n", message->current);
		message->messageType = Reply;
		
		len = sendto(sockfd, message, sizeof(*message), 0, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
		
		if (len == -1) {
			perror("Failed to send\n");
			close(sockfd);
			exit(EXIT_FAILURE);
		}
	}
	
	close(sockfd);
	return 0;
}
