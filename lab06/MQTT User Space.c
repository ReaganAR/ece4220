/* 	Name       : 	server_udp_broadcast.c
	Author     : 	Luis A. Rivera
    Modified By:    Ramy Farag
	Description: 	Lab6
					ECE4220/7220		*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <errno.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>

#include <mosquitto.h>
#define MSG_SIZE 40			// message size
char buffer[MSG_SIZE];	// to store received messages or messages to be sent.
char bufferPrev[MSG_SIZE];	// to store received messages or messages to be sent.
	


int matchString1;		//to match WHOIS
int matchString2;		//to match VOTE
int match1;			// to match notes 
int match2;
int match3;
int match4;
int match5;

int count1 = 0;
int master=0;

int cdev_id;


static char receive[MSG_SIZE];

#define CHAR_DEV "/dev/Lab6" // "/dev/YourDevName"


struct ifreq ifr;

int rand1 ;

char hostIp[100];
int IP[6],OtherIP[6],k=0;
char *token1,*token2;


struct mosquitto *mosq;

void on_connect(struct mosquitto *mosq, void *obj, int rc) {
	printf("ID: %d\n", * (int *) obj);
	if(rc) {printf("Error with result code: %d\n", rc);exit(-1);}
	mosquitto_subscribe(mosq, NULL, "Election/E1/E2", 0);
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {

	printf("New message with topic %s: %s\n", msg->topic, (char *) msg->payload);
    strcpy(buffer,(char *) msg->payload);

    matchString1 = strncmp(buffer,"WHOIS",5);		//comparing receiving message with 	"WHOIS"
    matchString2 = strncmp(buffer,"VOTE",4);		//comparing receiving message with 	"VOTE"
    match1 = strncmp(buffer,"@A",2);
    match2 = strncmp(buffer,"@B",2);
    match3 = strncmp(buffer,"@C",2);				//comparing receieved message with notes.
    match4 = strncmp(buffer,"@D",2);
    match5 = strncmp(buffer,"@E",2);



    if((match1 == 0)||(match2 == 0)||(match3 == 0)||(match4 == 0)||(match5 == 0))
    {
        int  dummy = write(cdev_id, buffer, sizeof(buffer));		//If any of the notes matches the buffer, then it that would be written onto the kernel module.
        printf("Printing to Kernel Through the Driver %s\n",buffer);

        if(dummy != sizeof(buffer)) {printf("Write failed, leaving...\n");}


        if((master == 1) && (strncmp(buffer,bufferPrev,2)) !=0  ) 
        { mosquitto_publish(mosq, NULL, "Election/E1/E2", sizeof(buffer), buffer, 0, false);
        }

    }


    if(matchString1 == 0)
		{
			if (master == 1)
			{
				char *msg1= (char*)malloc(sizeof(char)*40);
				strcpy(msg1,"Ramy is master on ");

				strcat(msg1,inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
				printf("Message To Broadcast: %s \n",msg1);

				mosquitto_publish(mosq, NULL, "Election/E1/E2", MSG_SIZE, msg1, 0, false);
				free(msg1);

			}
		}


    if(matchString2 == 0)	//If VOTE is the receieved message than it will broadcast host ip onto the network and set j=1.
		{
            
			char *msg2= (char*)malloc(sizeof(char)*20);
            
			strcat(msg2,"# ");
			strcat(msg2,inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
            strcat(msg2," ");
            

            
			rand1 = rand()%10;
            char random1[2];

			sprintf(random1,"%d",rand1);
            strcat(msg2,random1);
			
            strcat(msg2,"\n");
			printf("Send msg: %s\n",msg2);

            mosquitto_publish(mosq, NULL, "Election/E1/E2", MSG_SIZE, msg2, 0, false);
			
			master = 1;

			free(msg2);
		}


    if(buffer[0]=='#' && master == 1)
		{
			char *msg3= (char*)malloc(sizeof(char)*40);
			strcpy(msg3,buffer);
			token1 = strtok(msg3," .");
            int i=0;
			while(token1)
			{
				//	printf("token:%s\n",token1);
				OtherIP[i] = atoi(token1);
				token1 = strtok(NULL," .");
				i++;
			}
			//   printf("num1:%d , num2:%d \n", num1[5],num1[4]);
			if(OtherIP[5]<rand1)
			{
				master = 1;
			}
			else if(OtherIP[5] == rand1)
			{
				if(OtherIP[4]<IP[3])
				{
					master = 1;
				}

				else if(OtherIP[4] == IP[3])
				{
					master = 1;
				}
				else
				{
					master = 0;
				}
			}
			else
			{
				master = 0;
			}
			free(msg3);

		}



    strcpy(bufferPrev,buffer);

    
}







void ThreadJob1(void *ptr)			
{
	while(1)
	{
		int dummy2 = read(cdev_id, receive, MSG_SIZE);			// Here, user space program would read the note from chracter device.		
        if(dummy2 != sizeof(receive)) {	printf("Read failed, leaving...\n"); break;}
        
        if (receive[0]!='\0')
            {printf("Message from kernel space: %s\n", receive);
		    printf("dummy2=%d\n",dummy2);

            if(master == 1)		// if my server is master, than it would broadcast the note.
			{
                mosquitto_publish(mosq, NULL, "Election/E1/E2", MSG_SIZE, receive, 0, false);
			}


            }



	}
	pthread_exit(0);
}

int main(int argc, char *argv[])
{

srand(time(0));

int sock = socket(AF_INET, SOCK_DGRAM, 0); // Creates socket. Connectionless.
ifr.ifr_addr.sa_family = AF_INET;
strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ-1);
ioctl(sock, SIOCGIFADDR, &ifr);



strcpy(hostIp,inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
token2 = strtok(hostIp,".");

while(token2)
{
    IP[k] = atoi(token2);
    token2 = strtok(NULL,".");
    k++;
}
printf("last digit:%d \n", IP[3]);




if((cdev_id = open(CHAR_DEV, O_RDWR)) == -1) {
    printf("Cannot open device %s\n", CHAR_DEV);
    exit(1);
}





int rc, id=12;
mosquitto_lib_init();

mosq = mosquitto_new("subscribe-1", true, &id);
mosquitto_connect_callback_set(mosq, on_connect);
mosquitto_message_callback_set(mosq, on_message);
char *host ="128.206.19.16";
rc = mosquitto_connect(mosq, host, 1883, 10);
if(rc) {
    printf("Could not connect to Broker with return code %d\n", rc);
    return -1;
}
mosquitto_loop_start(mosq);


pthread_t thread1;
pthread_create(&thread1, NULL, (void *)&ThreadJob1, NULL);
pthread_join(thread1, NULL);
}
