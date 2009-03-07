#include <stdio.h>		/*for printf() and fprintf()*/
#include <sys/socket.h>		/*for socket(), bind() and connect()*/
#include <arpa/inet.h>		/*for sockaddr_in() and inet_ntoa()*/
#include <stdlib.h>		/*for atoi()*/
#include <string.h>		/*for memset()*/
#include <unistd.h>		/*for close()*/
#include <sys/wait.h>		/*for waitpid()*/
#include <sys/types.h>
#include <pthread.h>
#include <iostream>
#include <string>
#include "messagehandle.h"
#include "../db/database.h"
#include "strparse.h"

using namespace std;

#define MAXPENDING 5   /*the maximum number of connetion requests*/
#define RCVBUFFERSIZE 255

struct TCPinfo
{
  int sock;
  string ip_address;
};

// Helper Functions
string receive_TCP_message( int );
void send_TCP_message( int, const char* );

// Initializer functions
void add_server_ip(char*, int);
void get_server_list(char*, int);
void inform_other_servers();

// Thread Functions
void* handle_TCP( void* );
void* ping_check( void* );

database mydb;

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

  if(argc!=4)
    {
      fprintf(stderr,"Usage : %s <Central Server> <Central Port> <Server port>\n",argv[0]);
      exit(1);
    }
  
  cout << "Initializing database...\n";
  mydb.db_init();
  cout << "[OK]\n";
  
  echoServPort=atoi(argv[3]);

  if( echoServPort != 5000 )
    {
      printf("Warning: server should run on port 5000\n");
    }

  cout << "Getting server list from central sever...";
  get_server_list( argv[1], atoi(argv[2]) );
  add_server_ip( argv[1], atoi(argv[2]) );
  cout << "[OK]\n";

  cout << "Informing other Servers...\n";
  inform_other_servers();
  cout << "[OK]\n";

  // break off ping checker thread
  pthread_attr_init( &attr );
  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
  pthread_create(&thread,&attr,ping_check,NULL);
  
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
  
  TCPinfo info;
  for(;;)
    {
      clntLen=sizeof(echoClntAddr);
      if((clntSock=accept(servSock,(struct sockaddr *)&echoClntAddr,(socklen_t *)&clntLen))<0)
	{
	  perror("accept() failed");
	  exit(1);
	}
      
      //printf("Handling client: %s\n",inet_ntoa(echoClntAddr.sin_addr));
      info.sock = clntSock;
      info.ip_address = inet_ntoa(echoClntAddr.sin_addr);
      
      pthread_attr_init( &attr );
      pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
      pthread_create(&thread,&attr,handle_TCP,(void *)&info);
    }
  
  return 0;
}

string receive_TCP_message( int sock )
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

void* handle_TCP( void* arg )
{
  TCPinfo info = *((TCPinfo*)arg);
  int sock = info.sock;
  string message = receive_TCP_message(sock);
	
  // handle the message
  messagehandle m(message);
  m.set_socket(sock);
  m.set_IPaddress(info.ip_address);
  m.parse_message();
  
  close(sock);
  pthread_exit(NULL);
}

//every minute check servers are still connectd
void* ping_check( void* arg )
{
  arg = NULL;
  database mydb;
  int sock;			/*Socket discriptor*/
  struct sockaddr_in echoServAddr;/*the adress of the server socket*/
  int num_ips;
  string ip_to_check;

  while(true)
    {
      cout << "pinging other servers...\n";

      mydb.clearBuffer();
      num_ips = mydb.get_server_ip_list();

      for( int i = 0; i < num_ips; i++ )
	{
	  ip_to_check = mydb.getBuffer(i);
	  if((sock=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
	    {
	      perror("socket() failed in function ping_check()");
	      exit(1);
	    }

	  /*Configure the server*/
	  memset(&echoServAddr,0,sizeof(echoServAddr));
	  echoServAddr.sin_family=AF_INET;
	  echoServAddr.sin_port=htons(5000); // server will always use port 5000
	  echoServAddr.sin_addr.s_addr=inet_addr( ip_to_check.c_str() );
	
	  /*Establish the connection*/
	  if(connect(sock,(struct sockaddr *)&echoServAddr,sizeof(echoServAddr))<0)
	    {
	      cout << ip_to_check << " is no longer alive.\n";
	      mydb.deleteServerIP(ip_to_check );	      
	    }
	  else
	    {
	      send_TCP_message(sock, "ping~");
	      string ret = receive_TCP_message(sock);
	      if( ret != "pong" )
		{
		  cout << "Warning: expected message 'pong'\n" <<"but received message'"
		       << ret << "' from ip: "<< ip_to_check << endl;
		}
	    }

	        
      close(sock);
    }
      sleep(60);
    }
}

void add_server_ip(char* servIP, int echoServPort)
{
  int sock;			/*Socket discriptor*/
  char *echoString;		/*String to be echoed*/
  struct sockaddr_in echoServAddr;/*the adress of the server socket*/

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

  echoString = "add_serv~";
  send_TCP_message(sock, echoString);
  string response = receive_TCP_message(sock);

  if( response != "success" )
    {
      cout << "problem sending server ip\n";
      exit(1);
    }

  close(sock);
}

void get_server_list(char* servIP, int echoServPort)
{
  int sock;			/*Socket discriptor*/
  struct sockaddr_in echoServAddr;/*the adress of the server socket*/
  int bytesRcv;
  char echoBuffer[RCVBUFFERSIZE];

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

  send_TCP_message(sock, "req_serv~");
  string iplist = "";
  do
    {
      if((bytesRcv=recv(sock,echoBuffer,RCVBUFFERSIZE-1,0))<0)
	{
	  perror("recv() failed");
	  exit(1);
	}
      
      echoBuffer[bytesRcv]='\0';
      iplist += echoBuffer;
    }while(iplist.substr(iplist.find_last_of("<"),6) != "<done>" );

  close(sock);

  strparse parse(iplist);
  string token;
  int num_ip_addresses = atoi(parse.get_str_between("[","]").c_str() );

  cout << endl;
  for( int i = 0; i < num_ip_addresses; i++ )
    {
      token = parse.get_str_between("<",">");
      mydb.insert_server_ip( token );
    }

}

void inform_other_servers()
{
  // get all the server IP addresses
  mydb.clearBuffer();
  int num_ips = mydb.get_server_ip_list();

  int sock;			/*Socket discriptor*/
  struct sockaddr_in echoServAddr;/*the adress of the server socket*/
  for( int i = 0; i < num_ips; i++ )
    {
      if((sock=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
	{
	  perror("socket() failed in function inform_other_servers()");
	  exit(1);
	}

      /*Configure the server*/
      memset(&echoServAddr,0,sizeof(echoServAddr));
      echoServAddr.sin_family=AF_INET;
      echoServAddr.sin_port=htons(5000); // server will always use port 5000
      echoServAddr.sin_addr.s_addr=inet_addr( mydb.getBuffer(i).c_str() );
	
      /*Establish the connection*/
      if(connect(sock,(struct sockaddr *)&echoServAddr,sizeof(echoServAddr))<0)
	{
	  mydb.deleteServerIP(mydb.getBuffer(i));
	}
      else
	{
	  send_TCP_message(sock, "add_server~");
	  string ret = receive_TCP_message(sock);

	  if( ret != "add_server~success" )
	    {
	      cout << "Warning: expected message 'pong'\n" <<"but received message'"
		   << ret << "' from ip: "<< mydb.getBuffer(i) << endl;
	    }
	}
      
      close(sock);
    }
}
