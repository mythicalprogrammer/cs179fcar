#include <stdio.h>          /*for printf() and fprintf() */
#include <sys/socket.h>     /*for socket(), connect(), send(), recv()*/
#include <arpa/inet.h> 	    /*for sockaddr_in and inet_addr()*/
#include <stdlib.h> 	    /*for atoi()*/
#include <string.h>	    /*for memset()*/
#include <unistd.h>	    /*for close()*/

#define RCVBUFFERSIZE 32    /* Size of the receive buffer*/

int main(int argc,char *argv[])
{

	int sock;			/*Socket discriptor*/
	char *servIP;			/*Server's IP*/
	char *echoString;		/*String to be echoed*/
	int echoServPort;		/*the port of the server*/
	struct sockaddr_in echoServAddr;/*the adress of the server socket*/
	int echoStringLen;		/*the length of the echo string*/
	int bytesRcv;
	char echoBuffer[RCVBUFFERSIZE];

	/*Checking for wrong arguments*/
	if((argc<3) || (argc>4)) 
	{
		fprintf(stderr,"Usage: %s <ServerIP> <Echo Word> [<Echo Port>]\n",argv[0]);
		exit(1);
	}
	
	servIP=argv[1];  
	echoString=argv[2];

	if(argc==4)
		echoServPort=atoi(argv[3]);
	else
		echoServPort=7;  /*the default echo server port*/

	/* Creation of the socket*/
	if((sock=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
	{
		perror("socket() failed");
		exit(1);
	}

	/*Configure the server*/
	memset(&echoServAddr,0,sizeof(echoServAddr));
	echoServAddr.sin_family=AF_INET;
	echoServAddr.sin_port=htons(echoServPort);
	echoServAddr.sin_addr.s_addr=inet_addr(servIP);
	
	/*Establish the connection*/
	if(connect(sock,(struct sockaddr *)&echoServAddr,sizeof(echoServAddr))<0)
	{
		perror("connect() failed");
		exit(1);
	}
	
	echoStringLen=strlen(echoString);
	//system("sleep 20");
	
	/*Sending of the data*/
	if(send(sock,echoString,echoStringLen,0)!=echoStringLen)
	{
		perror("send() failed");
		exit(1);
	}

	/*Receive the same string back from the echo server*/
	printf("Received back from the server: ");
	do
	{
		if((bytesRcv=recv(sock,echoBuffer,RCVBUFFERSIZE-1,0))<0)
		{
			perror("recv() failed");
			exit(1);
		}
		
		echoBuffer[bytesRcv]='\0';
		printf(echoBuffer);
		
	}while(bytesRcv < 0 );
	
	printf("\n");
	close(sock);
	exit(0);
}

	
