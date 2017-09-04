/*
** listener.c -- a datagram sockets "server" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXBUFLEN 100

struct __attribute__((__packed__)) Message {
	short len;
	short id;
	char op;
	char message[MAXBUFLEN - 5];
} recMessage;

struct __attribute__((__packed__)) Response1 {
	short len;
	short id;
	unsigned char answer;
} lenMessage;

struct __attribute__((__packed__)) Response2 {
	short len;
	short id;
	char answer[MAXBUFLEN - 5];
} removeMessage;

struct __attribute__((__packed__)) Response3 {
	short len;
	short id;
	char answer[MAXBUFLEN - 5];
} upperMessage;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	short request;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	char * myPort;

	if (argc != 2) {
		fprintf(stderr,"usage\n");
		exit(1);
	}

	myPort = argv[1];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, myPort, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	request = 1;
	while(1)
	{

		printf("listener: waiting to recvfrom...\n");

		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		buf[numbytes] = '\0';

		recMessage = *((struct Message *)buf);

		if (recMessage.op == (char)5)
		{
			short count = 0;
			int i = 0;
			while (i < strlen(recMessage.message))
			{
				char current = toupper(recMessage.message[i]);
				switch (current) {
                    case 'A':
                    case 'E':
                    case 'I':
                    case 'O':
                    case 'U':
						break;
                    default:
                        count++;
                        break;
				}
				i++;
			}

			lenMessage.id = request;
			lenMessage.answer = (unsigned char) count;
			lenMessage.len = 6;

			if ((numbytes = sendto(sockfd, (char*)&lenMessage, lenMessage.len, 0,
				 (struct sockaddr *)&their_addr, addr_len)) == -1) {
			    perror("listener: sendto\n");
			    exit(1);
			}
		}
        else if (recMessage.op == (char)10)
        {
            char newArray[strlen(recMessage.message)];
            short index = 0;
            int i = 0;

            while (i < strlen(recMessage.message))
            {
                char current = recMessage.message[i];
                newArray[index] = toupper(current);
                index++;
                i++;
            }

            upperMessage.id = request;
            strcpy(upperMessage.answer, newArray);
            upperMessage.len = 4 + index;

            if ((numbytes = sendto(sockfd, (char*)&upperMessage, upperMessage.len, 0,
                            (struct sockaddr *)&their_addr, addr_len)) == -1)
            {
                perror("listener: sendto\n");
                exit(1);
            }
        }
		else if (recMessage.op == (char)170)
		{
			char newArray[strlen(recMessage.message)];
			short index = 0;
			int i = 0;
			while (i < strlen(recMessage.message))
			{
				char current = recMessage.message[i];
				switch (toupper(current)) {
					case 'A':
					case 'E':
					case 'I':
					case 'O':
					case 'U':
						break;
					default:
						newArray[index] = current;
						index++;
						break;
				}

				i++;
			}

			removeMessage.id = request;
			strcpy(removeMessage.answer, newArray);
			removeMessage.len = 4 + index;

			if ((numbytes = sendto(sockfd, (char*)&removeMessage, removeMessage.len, 0,
				 (struct sockaddr *)&their_addr, addr_len)) == -1) {
			    perror("listener: sendto\n");
			    exit(1);
			}
		}
		else
		{
			printf("Invalid Operation\n");
		}

		request++;
	}

	close(sockfd);

	return 0;
}
