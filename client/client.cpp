#include <stdio.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/stat.h>		/*Required for file stat */
#include <fcntl.h> 		/*Required for open() function */
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#define MAXPENDING 10
#define RCVBUFFERSIZE 255

int send_file(int port)
{
	int servSock; 		/*Socket descriptor for server*/
	int clntSock;		/*Socket descriptor for client*/
	struct sockaddr_in echoServAddr;	/*Local address*/
	struct sockaddr_in echoClntAddr;	/*Client address*/
	unsigned short echoServPort;	/*Server port*/
	unsigned int clntLen;	/*Length of the client address data structure*/
	struct stat stat_buf;
	off_t offset = 0;
	int rc;
	int fd;
	int dc;
	char filename[RCVBUFFERSIZE];
	echoServPort = port;
	//Create socket for incoming connections
	if((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		cout << "Unable to create socket for incoming connection\n";
	//Construct local address structure
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	echoServAddr.sin_port = htons(echoServPort);
	//Bind to local address
	if(bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
		cout << "Unable to bind to local address\n";
	//Listen for incoming connection
	if(listen(servSock, MAXPENDING) < 0)
		cout << "Unable to listen for incoming connections\n";
	for(;;)
	{	
		sleep(1);
		clntLen = sizeof(echoClntAddr);
		//Wait for client to connect
		clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen);
		if(clntSock < 0)
			cout << "Unable to accept client connection\n";
		printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
		//Receive filename of the file the client wants to get
		rc = recv(clntSock,filename, RCVBUFFERSIZE-1, 0);
		filename[RCVBUFFERSIZE] = '\0';
		printf( "Filename: %s\n", filename);
		if(rc < 0)
		{
			perror("Failed to received filename requested");
			exit(1);
		}
		else if(rc >= 0)
		{
			/*
			//Strip newline from filename
			for(unsigned int i = 0; i < 20; i++)
			{
				if(filename[i] == '\n' || filename[i] == '\r' || filename[i] == '"' || filename[i] == '?')
					filename[i] = '\0';
			}
			*/
			cout << "Received filename request\n";
			//open the file to be send
			fd = open("cleaner.exe", O_RDWR);
			if(fd == -1)
			{
		  		cout << "Unable to open: " << filename;
		  		exit(1);
			}
			//Create the file stat
			fstat(fd, &stat_buf);
			int filesize = stat_buf.st_size;
			char temp[20];
			string s;
			stringstream out;
			out << filesize;
			s = out.str();
			strcpy(temp, s.c_str());
			//Send the filesize to the client
			cout << "The size of the file being requested is " << temp << "\n";
			int sendLen = strlen(s.c_str() );
			dc=send(clntSock, s.c_str() , sendLen, 0);		
			if(dc < 0)
			{
				perror("send filesize error");
				exit(1);
			}
			cout << "here\n";
			//sleep(1);
			rc = sendfile(clntSock,fd,&offset, stat_buf.st_size);
			if(rc < 0)
			{
				perror("sendfile error");
				exit(1);
			}
			cout << "there\n";
			//Check to see if all the data has been correctly transferred to the client
			if(rc !=  stat_buf.st_size)
			{	
				fprintf(stderr, "incomplete transfer: %d of %d bytes\n", rc, (int)stat_buf.st_size);
				exit(1);
			}
		}
	}
	close(servSock);
	close(clntSock);
	close(fd);
	return 1;
}
int receivefile(char *IP, int ServPort, string filerequested)
{
	int sock;		/*Socket Descriptor*/
	struct sockaddr_in echoServAddr;	/*Echo server address*/
	unsigned short echoServPort = ServPort;	/*Server port number*/
	char *servIP = IP;		/*Server IP address*/
	int bytesRcvd;	/*# of bytes received during file transfer*/
	int totalBytesRcvd;		/*Total filesize of file being transmitted*/
	char echoBuffer[RCVBUFFERSIZE];	/*Buffer to hold the filesize*/	
	string filename = filerequested;
	int filesize;
	
	char * fileBuffer;
	//Create a file with the same filename to write to
	int filetoreceive = open("cleaner1.exe", O_RDWR|O_CREAT|O_EXCL, S_IREAD|S_IWRITE);
	//Create a stream socket using TCP
	if((sock = socket(PF_INET, SOCK_STREAM,IPPROTO_TCP)) < 0)
		cout << "Unable to create a TCP socket\n";
	//Construct the server address structure
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_addr.s_addr = inet_addr(servIP);
	echoServAddr.sin_port = htons(echoServPort);
	//Establish the connection to the echo server
	if(connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
		cout << "Unable to establish an connection with the server\n";
	char temp[20];
	strcpy(temp, filename.c_str());
	//Send the requested file to the client that has the file
	int n = send(sock, temp, strlen(temp), 0);
	if(n<0)
		perror("Error sending file requested");
	bytesRcvd = recv(sock, echoBuffer, RCVBUFFERSIZE-1,0); 	//Get the filesize from client
	echoBuffer[bytesRcvd] = '\0';
	cout << "Received filesize: " << echoBuffer << "\n";
	filesize = atoi(echoBuffer);
	fileBuffer = (char *) malloc(sizeof(char)*filesize);
	totalBytesRcvd = 0;
	while(totalBytesRcvd < filesize )
	{
		cout << "Receiving file\n";
		//Check to see if the socket is receiving the bytes
		if((bytesRcvd = recv(sock, fileBuffer,filesize, 0)) <= 0)
		{
			cout << "Failed connection\n";
			exit(1);
		}
		else
		{	
			//Write to the file
			write(filetoreceive, fileBuffer, bytesRcvd);
			totalBytesRcvd = totalBytesRcvd + bytesRcvd;
		}
		cout << "Total bytes received " << totalBytesRcvd << "\n";
	}
	close(sock);
	exit(0);
	return 1;
}

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

	
