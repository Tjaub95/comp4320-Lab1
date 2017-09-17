/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>


#define BACKLOG 10	 // how many pending connections queue will hold

#define MAXDATASIZE 100

struct __attribute__((__packed__)) Message {
    short len;
    short id;
    char op;
    char message[MAXDATASIZE - 5];
} recMessage;

struct __attribute__((__packed__)) Response1 {
    short len;
    short id;
    short answer;
} lenMessage;

struct __attribute__((__packed__)) Response2 {
    short len;
    short id;
    char answer[MAXDATASIZE - 5];
} removeMessage;

struct __attribute__((__packed__)) Response3 {
    short len;
    short id;
    char answer[MAXDATASIZE - 5];
} upperMessage;

void displayBuffer(char *Buffer, int length); // _M3

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

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
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
    short request;
	struct sigaction sa;
	int yes=1;

	char s[INET6_ADDRSTRLEN];
	int rv;

	int numbytes;
	char buf[MAXDATASIZE];


	if (argc != 2) {
		fprintf(stderr,"usage: TCPServerDisplay Port# \n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, argv[1] /* _M1 PORT*/, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

    request = 1;
	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) {
		  close(sockfd);
		  if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
		    perror("recv");
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

              if (numbytes = send(new_fd, (char*)&lenMessage, lenMessage.len, 0) == -1)
              {
                  perror("listener: send\n");
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

            if (numbytes = send(new_fd, (char*)&upperMessage, upperMessage.len, 0) == -1)
            {
                perror("listener: send\n");
                exit(1);
            }
          }
		  else if (recMessage.op == (char)80)
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

			if (numbytes = send(new_fd, (char*)&removeMessage, removeMessage.len, 0) == -1) {
			    perror("listener: send\n");
			    exit(1);
			}
		  }
          else
          {
             printf("Invalid Operation\n");
          }
          request++;

		  close(new_fd);
		  exit(0);
		}
		close(new_fd);
	}

	return 0;
}
