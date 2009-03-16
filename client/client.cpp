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
#include <stddef.h>
#include <dirent.h>
using namespace std;

#define MAXPENDING 10
#define RCVBUFFERSIZE 255
int send_file(int port);
int receive_file(char *IP, int ServPort, string filerequested);
int search_file(string file);
int command(char *serverIP, int serverPort, char *command); 
int add_file(char *servIP, int echoServPort);

int main(int argc,char *argv[])
{
	int action;
	char *addclient = "add_client~";
	char *reqfile = "req_file_clnt~";
	string filename;
	char *servIP;			/*Server's IP*/
	int echoServPort;		/*the port of the server*/

	/*Checking for wrong arguments*/
	if((argc<2)) 
	{
		fprintf(stderr,"Usage: %s <ServerIP> <Echo Port>\n",argv[0]);
		exit(1);
	}
	
	servIP=argv[1];  
	echoServPort=5000;  /*the default echo server port*/
	
	printf("**************CAR****************\n");
	printf("*********************************\n");
	printf("1) Connect to Server\n");
	printf("2) Quit\n");
	printf("Please select the corresponding number to what you want to do: ");
	cin >> action;
	if(action == 1)
	{
		command(servIP, echoServPort, addclient);
		printf("*********************************\n");
		printf("1) Search for a file\n");
		printf("2) Add a file\n");
		printf("Please select the corresponding number to what you want to do: ");	
		int action2;
		cin >> action2;
		if(action2 == 1)
		{
			printf("What is the name of the file that you want to search for? ");
			cin >> filename;
			string req = (string)reqfile + "[" + filename + "]";
			char *ptr = &req[0];
			command(servIP, echoServPort, ptr);
		}
		if(action2 == 2)
		{
			printf("Adding all files located in the shared folder\n");
			add_file(servIP, echoServPort);
		}
	}
	else if(action == 2)
	{
		exit(1);
	}
	else
	{
		cout << "Option not recognized\n";
		cout << "Please enter again\n";
		cin >> action;
	}
}

int add_file(char *servIP, int echoServPort)
{
	char *addfile = "add_file~";
	DIR *dp;
	struct dirent *ep;
	dp = opendir("shared");
	if(dp != NULL)
	{
		while((ep = readdir(dp)))
		{
			if(((string)(ep->d_name) != ".") or ((string)(ep->d_name) != ".."))
			{
				cout << (string)(ep->d_name) << "\n";
				string req = (string)addfile + "[" + (string)(ep->d_name) + "]";
				char *ptr = &req[0];
				command(servIP, echoServPort, ptr);
			}
		}
		closedir(dp);
	}
	return 1;
}
int command(char *serverIP, int serverPort, char *command)
{
	int sock;			/*Socket discriptor*/
	char *servIP = serverIP;			/*Server's IP*/
	int echoServPort = serverPort;		/*the port of the server*/
	struct sockaddr_in echoServAddr;/*the adress of the server socket*/
	char status[RCVBUFFERSIZE];
	int msg;
	if((sock=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
		{
			perror("socket() failed");
			exit(1);
		}
		//Configure the server
		memset(&echoServAddr,0,sizeof(echoServAddr));
		echoServAddr.sin_family=AF_INET;
		echoServAddr.sin_port=htons(echoServPort);
		echoServAddr.sin_addr.s_addr=inet_addr(servIP);
	
		//Establish the connection
		if(connect(sock,(struct sockaddr *)&echoServAddr,sizeof(echoServAddr))<0)
		{
			perror("connect() failed");
			exit(1);
		}
		if(send(sock,command,strlen(command), 0) < 0)
		{
			perror("Error command not recognized");
		}
		//Receive add client success message from server
		msg = recv(sock, status, RCVBUFFERSIZE-1, 0);
		if(msg < 0)
		{
			perror("Cannot display server message\n");
		}
		status[msg] = '\0';
		cout << status << "\n";
	close(sock);
	return 1;
}
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
int receive_file(char *IP, int ServPort, string filerequested)
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
