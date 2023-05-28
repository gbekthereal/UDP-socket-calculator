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

    unsigned int id;                    /* unique identifier */

    unsigned int procedureId;              /* e.g. (1, 2, 3, 4) for (+, -, *, /) */

    int arg1;                              /* argument/ return parameter */

    int arg2;                              /* argument/ return parameter */

    int continuation;

    int current;

    int close;

} Message;                          /* each int (and unsigned int) is 32 bits = 4 bytes */

void builder(int current, Message * message, int a, int b, char op, int id, int opIndex) {
	message->messageType = Request;
	message->id = (unsigned int)id;
	
	if (opIndex == 0) {			/* if operator is at [0] then continue from last solution */
		message->continuation = 1;
		message->arg1 = current;	/* current is the variable fill with the server response at the end */
	}
	else {
		message->arg1 = a;
	}
	
	message->arg2 = b;
	
	if (op == '+')	/* assigning procedureID based on operator */
		message->procedureId = 1;
	else if (op == '-')
		message->procedureId = 2;
	else if (op == '*')
		message->procedureId = 3;
	else if (op == '/'){
		message->procedureId = 4;
	}
	else {
		printf("ERROR in input formatting");
		exit(0);
	}
}



int main() {
	struct sockaddr_in servaddr = {0};
	
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);  // UDP socket declaration
	if (sockfd == -1) {
		perror("Failed to create socket\n");
		exit(EXIT_FAILURE);
	}
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(8080);
	servaddr.sin_addr.s_addr = INADDR_ANY;
	
	Message * message = malloc(sizeof(Message)); // buffer for in/out messages
	Message * reply = malloc(sizeof(Message));
	
	for (;;) {
		int x, y;
		printf("Enter Expression ('exit' to close):");
		char * expression = malloc(MAX);	// hold message from user
		scanf("%s", expression);		// get message from user 
		
		if (strcmp(expression, "exit") == 0)
			break;
	
		/* INPUT FORMATTING */
		char op;
		int opIndex;
		
		for (int i = 0; i < sizeof(expression); i++) {
			if (expression[i] == '+' || expression[i] == '-' || expression[i] == '*' || expression[i] == '/') {
				op = expression[i];
				opIndex = i;
			}
		}
		
		/* Taking the substrings that represent each part of the expression */
		char * operand1 = malloc(MAX);
		char * operand2 = malloc(MAX);
		
		if (opIndex == 0)
			x = reply->current;
		else {
			for (int i = 0; i < opIndex; i++){
				if (i != opIndex)
					strcat(operand1, &expression[i]);     // another option to get substring
			}
			x = atoi(operand1);
		}
		for (int i = opIndex; i < sizeof(expression); i++){
		    if (i != opIndex)
		        strcat(operand2, &expression[i]);     // another option to get substring
		}
		y = atoi(operand2);
		
		builder(reply->current, message, x, y, op, 1, opIndex);
		
		/* SENDING THE MESSAGE VIA THE SOCKET */
		message->messageType = Request;
		socklen_t len = sendto(sockfd, message, sizeof(*message), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
		
		if (len == -1) {
			perror("Failed to send\n");
			close(sockfd);
			exit(EXIT_FAILURE);
		}
		
		/* SERVER RESPONSE */
		int n = recvfrom(sockfd, reply, sizeof(*reply), MSG_WAITALL, 0, &len);
		if (n == -1) {
			perror("Failed to receive\n");
			close(sockfd);
			exit(EXIT_FAILURE);
		}
		
		printf("Answer = %d\n", reply->current);
	}
	
	message->close = 1;
	socklen_t len = sendto(sockfd, message, sizeof(*message), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
	close(sockfd);
	return 0;
}
