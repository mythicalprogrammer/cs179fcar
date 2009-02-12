#include <stdio.h>		/*for printf() and fprintf()*/
#include <sys/socket.h>		/*for socket(), bind() and connect()*/
#include <arpa/inet.h>		/*for sockaddr_in() and inet_ntoa()*/
#include <stdlib.h>		/*for atoi()*/
#include <string.h>		/*for memset()*/
#include <unistd.h>		/*for close()*/
#include <sys/wait.h>	/*for waitpid()*/
#include <sys/types.h>
#include <pthread.h>
#include <iostream>
#include <string>

using namespace std;

#define MAXPENDING 5   /*the maximum number of connetion requests*/
#define RCVBUFFERSIZE 255

string recive_TCP_message( int );
void send_TCP_message( int, const char* );
void* handle_TCP( void* );

int main(int argc,char *argv[])
{

	int servSock;			/*the socket descriptor for the server socket*/
	int clntSock;			/*the socket descriptor for the client socket*/
	struct sockaddr_in echoServAddr;/*Local - Server address*/
	struct sockaddr_in echoClntAddr;/*Client Address*/
	int echoServPort;		/*Server Port*/
	int clntLen;			/*Length of Client address data structure*/
	pthread_t thread;
	pthread_attr_t attr;

	if(argc!=2)
	{
		fprintf(stderr,"Usage : %s <Server port>\n",argv[0]);
		exit(1);
	}

	echoServPort=atoi(argv[1]);

	if((servSock=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
	{
		perror("socket() failed");
		exit(1);
	}
	
	/*construct the local address for the server*/
	memset(&echoServAddr,0,sizeof(echoServAddr));
	echoServAddr.sin_family=AF_INET;
	echoServAddr.sin_port=htons(echoServPort);
	echoServAddr.sin_addr.s_addr=htonl(INADDR_ANY);

	/*bind the server socket*/
	if(bind(servSock,(struct sockaddr *)&echoServAddr,sizeof(echoServAddr))<0)
	{
		perror("bind() failed");
		exit(1);
	}
	
	/*make the server to listen for incoming connections*/
	if(listen(servSock,MAXPENDING)<0)
	{
		perror("listen() failed");
		exit(1);
	}
	
	for(;;)
	{
		clntLen=sizeof(echoClntAddr);
		if((clntSock=accept(servSock,(struct sockaddr *)&echoClntAddr,(socklen_t *)&clntLen))<0)
		{
			perror("accept() failed");
			exit(1);
		}
		
		printf("Handling client: %s\n",inet_ntoa(echoClntAddr.sin_addr));

		printf( "Old sock: %d \n", clntSock );
		pthread_attr_init( &attr );
		pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
		pthread_create(&thread,&attr,handle_TCP,(void *)clntSock);
	}
	
	return 0;
}

void* handle_TCP( void* arg )
{
	int sock = (int)arg;
	printf( "New sock: %d \n", sock );
	string message = recive_TCP_message(sock);
	cout << message << endl;
	send_TCP_message(sock,message.c_str());

	close(sock);
	pthread_exit(NULL);
}

string recive_TCP_message( int sock )
{
	int recvMsgSize;
	char echoBuffer[RCVBUFFERSIZE];
	string ret = "";
	
	do
	{
		if((recvMsgSize=recv(sock,echoBuffer,RCVBUFFERSIZE-1,0))<0)
		{
			perror("recv() failed");
			exit(1);
		}

		echoBuffer[recvMsgSize]='\0';
		ret += echoBuffer;
		
	}while(recvMsgSize < 0 );

	return ret;
}

void send_TCP_message( int sock, const char* message )
{
	int stringLen = strlen(message);
	
	if(send(sock,message,stringLen,0)!=stringLen)
	{
		perror("send() failed");
		exit(1);
	}
}


	
	
