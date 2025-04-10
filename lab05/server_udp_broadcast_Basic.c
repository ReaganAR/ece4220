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
#define BROADCAST_ADDR ""
#define 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sock, length, n;
    int boolval = 1;			// for a socket option
    socklen_t fromlen;
    struct sockaddr_in server;
    struct sockaddr_in addr;
    char buffer[MSG_SIZE];	// to store received messages or messages to be sent.
    bool isMaster = false;

    if (argc < 2)
    {
        printf("usage: %s port\n", argv[0]);
        exit(0);
    }

   sock = socket(AF_INET, SOCK_DGRAM, 0); // Creates socket. Connectionless.
   if (sock < 0)
	   error("Opening socket");

   length = sizeof(server);			// length of structure
   bzero(&server,length);			// sets all values to zero. memset() could be used
   server.sin_family = AF_INET;		// symbol constant for Internet domain
   server.sin_addr.s_addr = INADDR_ANY;		// IP address of the machine on which
											// the server is running
   server.sin_port = htons(atoi(argv[1]));	// port number

   // binds the socket to the address of the host and the port number
   if (bind(sock, (struct sockaddr *)&server, length) < 0)
       error("binding");

   // change socket permissions to allow broadcast
   if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &boolval, sizeof(boolval)) < 0)
   	{
   		printf("error setting socket options\n");
   		exit(-1);
   	}

   fromlen = sizeof(struct sockaddr_in);	// size of structure

   while (1)
   {
	   // bzero: to "clean up" the buffer. The messages aren't always the same length...
	   bzero(buffer,MSG_SIZE);		// sets all values to zero. memset() could be used

	   // receive from a client
	   n = recvfrom(sock, buffer, MSG_SIZE, 0, (struct sockaddr *)&addr, &fromlen);
       if (n < 0)
    	   error("recvfrom"); 

       printf("Received a datagram. It says: %s", buffer);

        // To send a broadcast message, we need to change IP address to broadcast address
        // If we don't change it (with the following line of code), the message
        // would be transmitted to the address from which the message was received.
        // You may need to change the address below (check ifconfig)
        addr.sin_addr.s_addr = inet_addr(BROADCAST_ADDR); //inet_addr("128.206.19.255");		// broadcast address

        // n = sendto(sock, "Got a message. Was it from you?\n", 32, 0,
        //             (struct sockaddr *)&addr, fromlen);
        // if (n  < 0)
        //     error("sendto");

        // Process incoming string
        if(strcmp(buffer, "WHOIS\n") == 0){
            printf("WHOIS received\n");
            if(isMaster){
                string sendMessage[MSG_SIZE] = "Reagan on board " + BROADCAST_ADDR + " is the master"
                sendto(sock, sendMessage, MSG_SIZE, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
                printf("Sending message: %s", sendMessage);
            }
        }
        else if(strcmp(buffer, "VOTE\n") == 0){
            printf("VOTE received\n");
            string sendMessage[MSG_SIZE] = '#' + BROADCAST_ADDR + ' ' + (rand() % 10 + 1);
            sendto(sock, sendMessage, MSG_SIZE, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
            printf("Sending message: %s", sendMessage);
        }
        else if(buffer[0] == '#'){
            printf("Vote number received\n");
        }

        
   }

   return 0;
 }
