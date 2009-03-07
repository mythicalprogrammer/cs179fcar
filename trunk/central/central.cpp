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
#include <sstream>
#include <string>
#include "../server/strparse.h"
#include "../db/cntrl_db.h"

using namespace std;

#define MAXPENDING 5   /*the maximum number of connetion requests*/
#define RCVBUFFERSIZE 255

struct ipinfo
{
  int sock;
  string ip;
};

string receive_TCP_message( int );
void send_TCP_message( int, const char* );
void* handle_TCP( void* );
void* handle_ping( void* );

cntrl_db mydb;

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
  ipinfo info;

  if(argc!=2)
    {
      fprintf(stderr,"Usage : %s <Server port>\n",argv[0]);
      exit(1);
    }
  
  // Initialize the database
  cout << "Initializeing database...\n";
  mydb.db_init();
  cout << "[OK]\n";

 // break off ping checker thread
  pthread_attr_init( &attr );
  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
  pthread_create(&thread,&attr,handle_ping,NULL);
	
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
      
      info.sock = clntSock;
      info.ip = inet_ntoa(echoClntAddr.sin_addr);
      //printf("Handling client: %s\n",inet_ntoa(echoClntAddr.sin_addr));
      
      pthread_attr_init( &attr );
      pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
      pthread_create(&thread,&attr,handle_TCP,(void* )&info);
    }
  
  return 0;
}

void* handle_TCP( void* arg )
{

  ipinfo info = *((ipinfo *)arg);

  int sock = info.sock;
  string message = receive_TCP_message(sock);
  
  strparse parse(message);
  string token = parse.get_next_token("~");
  
  // two options req_serv
  if( token == "req_serv" )
    {
      cout << "Handling req_serv from " << info.ip << endl;

      mydb.clearBuffer();
      int size = mydb.get_ip_list();

      stringstream out;
      out << size;      
      string ret = "ip_list~[" + out.str() + "]";

      for( int i = 0; i < size; i++ )
	ret += "<" + mydb.getBuffer(i) + ">";
      
      ret+="<done>";
      send_TCP_message(sock, ret.c_str() );
    }
  // add_serv
  else if( token == "add_serv" )
    {
      cout << "Handling add_serv from " << info.ip << endl;
      mydb.insert_ip(info.ip);
      
      send_TCP_message(sock, "success");
    }
  else
    {
      cout << "Unrecognized command: " + token + "\n";
      send_TCP_message(sock,"fail");
    }
  
  close(sock);
  pthread_exit(NULL);
}

void* handle_ping( void* arg )
{
  arg = NULL;
  int sock;			/*Socket discriptor*/
  struct sockaddr_in echoServAddr;/*the adress of the server socket*/
  int num_ips;
  string ip_to_check;

  while(true)
    {
      cout << "pinging list servers...\n";

      mydb.clearBuffer();
      num_ips = mydb.get_ip_list();

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
	      mydb.delete_server_IP(ip_to_check );	      
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
