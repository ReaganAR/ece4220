#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <sys/ioctl.h>
#include <net/if.h>

#define MSG_SIZE 40			// message size
char* BROADCAST_ADDR = "128.206.23.255";

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
    int isMaster = 0;
    int myVote = 0;

    // Vars for dynamic IP
    struct ifreq ifr;
    char ip[INET_ADDRSTRLEN];

    if (argc < 2)
    {
        printf("usage: %s port\n", argv[0]);
        exit(0);
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0); // Creates socket. Connectionless.
    if (sock < 0)
        error("Opening socket");

    /* Gather the IP of the local interface  - - - - - - - - - - - - - - - - - - - - - - - */
    // Specify the interface you're interested in
    strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ-1);
    ifr.ifr_name[IFNAMSIZ-1] = '\0';  // Just to be safe

    // Ioctl to get the IP address
    if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
        perror("ioctl(SIOCGIFADDR)");
        close(sock);
        return 1;
    }

    struct sockaddr_in *ipaddr = (struct sockaddr_in *)&ifr.ifr_addr;
    inet_ntop(AF_INET, &ipaddr->sin_addr, ip, sizeof(ip));
    printf("wlan0 IP address: %s\n", ip);


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
    char sendMessage[MSG_SIZE]; // Stores most recently sent message
    while (1)
    {
        // bzero: to "clean up" the buffer. The messages aren't always the same length...
        bzero(buffer,MSG_SIZE);		// sets all values to zero. memset() could be used

        // receive from a client
        n = recvfrom(sock, buffer, MSG_SIZE, 0, (struct sockaddr *)&addr, &fromlen);
        if (n < 0)
            error("recvfrom"); 

        printf("Received a datagram. It says: %s\n", buffer);

        // To send a broadcast message, we need to change IP address to broadcast address
        // If we don't change it (with the following line of code), the message
        // would be transmitted to the address from which the message was received.
        // You may need to change the address below (check ifconfig)
        addr.sin_addr.s_addr = inet_addr(BROADCAST_ADDR);


        // Process incoming string and verify we are not reading our previously sent message
        if(strcmp(buffer, sendMessage) != 0){   
            bzero(sendMessage,MSG_SIZE);

            if(strcmp(buffer, "WHOIS\n") == 0){ // WHOIS RECEIVED
                if(isMaster){
                    char sendMessage[MSG_SIZE] = "Reagan is master on ";
                    strcat(sendMessage, ip);
                    strcat(sendMessage, "\n");
                    sendto(sock, sendMessage, MSG_SIZE, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
                    printf("Sending message: %s", sendMessage);
                }
            }
            else if(strcmp(buffer, "VOTE\n") == 0){ // VOTE COMMAND RECEIVED
                strcpy(sendMessage, "# ");
                strcat(sendMessage, ip);
                char num[40];
                myVote = rand() % 10;
                sprintf(num, " %d\n", myVote); // Returns 0-9 inclusive
                strcat(sendMessage, num);
                sendto(sock, sendMessage, MSG_SIZE, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
                printf("Sending message: %s", sendMessage);
                isMaster = 1;
            }
            else if(buffer[0] == '#'){ // VOTE BALLOT RECEIVED
                printf("Vote Received: %d\tMy Vote: %d\n", buffer[17] - '0', myVote);
                if(buffer[17] - '0' == myVote) {
                    if(buffer[15] > BROADCAST_ADDR[13]){
                        isMaster = 0;
                        printf("Tied vote, relinquishing to higher IP\n");
                    }
                }
                if(buffer[17] - '0' > myVote) {
                    isMaster = 0;
                    printf("Lost vote, relinquishing to higher vote\n");
                }

            }
        }
        
    }

    return 0;
}
