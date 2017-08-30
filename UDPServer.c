/*
** listener.c -- a datagram sockets "server" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define BUF_SIZE 1024

const char *vowel_len_code = "85";
const char *disemvowel_code = "170";
const char *vowels = "aeiouAEIOU";
int count;

int main(int argc, char* argv[]) {
    unsigned char buf[BUF_SIZE];
    struct sockaddr_in self, other;
    int len = sizeof(struct sockaddr_in);
    int n, s, port;

    if (argc < 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    return 1;
    }

    /* initialize socket */
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    perror("socket");
    return 1;
    }

    /* bind to server port */
    port = atoi(argv[1]);
    memset((char *) &self, 0, sizeof(struct sockaddr_in));
    self.sin_family = AF_INET;
    self.sin_port = htons(port);
    self.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *) &self, sizeof(self)) == -1) {
    perror("bind");
    return 1;
    }

    while ((n = recvfrom(s, buf, BUF_SIZE, 0, (struct sockaddr *) &other, &len)) != -1) 
    {
    printf("Received message from client.", 
        inet_ntoa(other.sin_addr), 
        ntohs(other.sin_port)); 
    fflush(stdout);
     
  unsigned short messageLength = (buf[0] << 8) + buf[1];
  unsigned short requestNum = (buf[2] << 8) + buf[3];
  unsigned char operation = buf[4];
  unsigned char string[BUF_SIZE];
  int q = 0;
  int p;
  for (p = 5; buf[p] != '\0'; p++)
   {
     string[q] = buf[p];
     q++;
   }

      if (operation == 85)
       {   
          count = 0;
          int i = 0;
          n = 6;
          messageLength = 6;
          while(string[i] != '\0') 
          {
                if(string[i] == 'a' || string[i] == 'e'
                   || string[i] == 'i' || string[i] == 'o' || string[i] =='u'
                   || string [i] == 'A' || string[i] == 'E' || string[i] =='I'
                   || string [i] == 'O' || string[i] == 'U') 
                {
                count++;
                }
                i++;
          }   
          unsigned char countChar = (char)count;
          buf[5] = ((unsigned char)count);
          buf[4] = (unsigned char)(count >> 8);
          buf[1] = ((unsigned char)messageLength);
          buf[0] = (unsigned char)(messageLength >> 8);
       }
       
       else if (operation == 170)
       {    
           // printf("Hey");
           // fflush(stdout);
            int k = 0;
            unsigned char noVow[1024];
            int j = 5;
           
            int cons = 0;
            while(buf[j] != '\0' && j < messageLength) 
            {   
               if(buf[j] == 'a' || buf[j] == 'e' || buf[j] == 'i' || buf[j] == 'o'
                  || buf[j] == 'u' || buf[j] == 'A' || buf[j] == 'E' || buf[j] =='I'
                   || buf[j] == 'O' || buf[j] == 'U')
                {
                  buf[j-1] = buf[j+1];
                
                j++;
                

                }
              else  
              {
                   buf[j - 1] = buf[j];
                  j++;
                  cons++;
              }
                
            }
            messageLength = cons + 4;
            buf[4 + cons] = '\0';
            n =  cons + 4;
            buf[1] = ((unsigned char)messageLength);
            buf[0] = (unsigned char)(messageLength >> 8);
       }
   
 
    /* echo back to client */
    sendto(s, buf, n, 0, (struct sockaddr *) &other, len);
    }
    
    memset(buf, 0, sizeof buf);
    close(s);
    return 0;
}
