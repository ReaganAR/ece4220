/* 	Name       :   client_udp_broadcast.c
	Author     :   Luis A. Rivera
	Modified By:   Ramy Farag
	Description:   Simple client (broadcast)
	Course:        ECE4220/7220		*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MSG_SIZE 40			// message size

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
   int sock, n;
   unsigned int length;
   struct sockaddr_in anybody, from;
   char buffer[MSG_SIZE];	// to store received messages or messages to be sent.
   int boolval = 1;			// for a socket option

   if (argc != 2)
   {
	   printf("usage: %s port\n", argv[0]);
       exit(1);
   }

   sock = socket(AF_INET, SOCK_DGRAM, 0); // Creates socket. Connectionless.
   if (sock < 0)
	   error("socket");

   // change socket permissions to allow broadcast
   if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &boolval, sizeof(boolval)) < 0)
     	{
     		printf("error setting socket options\n");
     		exit(-1);
     	}

      anybody.sin_family = AF_INET;		// symbol constant for Internet domain
      anybody.sin_port = htons(atoi(argv[1]));// port field

      anybody.sin_addr.s_addr =  INADDR_ANY;  // Address of the server

	length = sizeof(struct sockaddr_in);		// size of structure

	// YOU have to bind to allow listening to broadcast
	if (bind(sock, (struct sockaddr *)&anybody, length) < 0)
		error("binding");



  do
  {

		  // receive messag
		  bzero(buffer,MSG_SIZE);		// "clean up" the buffer. sets all values to zero. memset() could be used
		  n = recvfrom(sock, buffer, MSG_SIZE, 0, (struct sockaddr *)&from, &length);

		  printf("Received: %s", buffer);

  } while(1);

   close(sock);						// close socket.
   return 0;
}
