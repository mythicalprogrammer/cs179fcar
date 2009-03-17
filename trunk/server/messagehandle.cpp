#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <string.h>
#include "../db/database.h"
#include "messagehandle.h"
#include "strparse.h"
#include <string>
#include <iostream>
#include <sstream>

using namespace std;
#define RCVBUFFERSIZE 255

void to_lower(string& s)
{
  for( unsigned i = 0; i < s.length(); i++ )
    if( (int)s[i] > 64 && (int)s[i] < 91 )
      s[i] = (char)( (int)s[i] + 32 );
}

string merge_ips( string s1, string s2 )
{
  string ret;
  strparse frst(s1);
  strparse scnd(s2);
  string token;

  int size_s1 = atoi( frst.get_str_between("[","]").c_str() );
  int size_s2 = atoi( scnd.get_str_between("[","]").c_str() );
  stringstream out;
  out <<  size_s1 + size_s2;

  ret = "[" + out.str() + "]";

  for( int i = 0; i < size_s1; i++ )
    {
      token = frst.get_str_between("<",">");
      ret += "<" + token + ">";
    }
  for( int i = 0; i < size_s2; i++ )
    {
      token = scnd.get_str_between("<",">");
      ret += "<" + token + ">";
    }

  ret += "<done>";

  return ret;
}

messagehandle::messagehandle()
{

}

messagehandle::messagehandle(string message)
{
	parse.add_string(message);
}

messagehandle::~messagehandle()
{

}

void messagehandle::set_socket(int i)
{
  socket_num = i;
}
void messagehandle::set_IPaddress(string ip)
{
  ip_address = ip;
}

void messagehandle::parse_message()
{
  string token;
	
  token = parse.get_next_token("~");
  to_lower(token);
	
  if( token == "ping" )
    ping();
  else if( token == "add_server" )
    add_server();
  else if( token == "add_client" )
    add_client();
  else if( token == "add_file" )
    add_file();
  else if( token == "add_file_list" )
    add_file_list();
  else if( token == "req_file_clnt" )
    req_file_clnt();
  else if( token == "req_file_serv" )
    req_file_serv();
  else
    {
      cout << "Error: could not recognize command\n";
    }
	
}


// #######################################################
// #######################################################
// ###                                                 ###
// ###                                                 ###
// ###          handle revieved messages               ###
// ###                                                 ###
// ###                                                 ###
// #######################################################
// #######################################################

void messagehandle::ping()
{
  cout << "Handling ping from " << ip_address << "\n";
  string message = "pong";
  int stringLen = strlen(message.c_str());
	
  if(send(socket_num,message.c_str(),stringLen,0)!=stringLen)
    {
      perror("send() failed");
      exit(1);
    }
}

void messagehandle::add_server()
{
  database mydb;
  mydb.insert_server_ip(ip_address);

  cout << "Server " + ip_address + " added.\n";

  string message = "add_server~success";
  int stringLen = strlen(message.c_str());
  if(send(socket_num,message.c_str(),stringLen,0)!=stringLen)
    {
      perror("send() failed");
      exit(1);
    }
}
void messagehandle::add_client()
{
  database mydb;
  mydb.insert_ip(ip_address);

  cout << "Client " + ip_address + " added.\n";

  string message = "add_client~success";
  int stringLen = strlen(message.c_str());
  if(send(socket_num,message.c_str(),stringLen,0)!=stringLen)
    {
      perror("send() failed");
      exit(1);
    }
}
void messagehandle::add_file()
{
  database mydb;
  string file = parse.get_str_between("[","]");
  mydb.insert_file(file, ip_address);

  string message = "add_file~success";
  int stringLen = strlen(message.c_str());
  if(send(socket_num,message.c_str(),stringLen,0)!=stringLen)
    {
      perror("send() failed");
      exit(1);
    }
}
void messagehandle::add_file_list()
{
  database mydb;

  int num_file = atoi( parse.get_str_between("<",">").c_str() );
  string file;

  for( int i = 0; i < num_file; i++ )
    {
      file = parse.get_str_between("[","]");
      cout << "Adding file '" << file << "' from " << ip_address << " to database\n";
      mydb.insert_file(file,ip_address);
    }

  string message = "add_file_list~success";
  int stringLen = strlen(message.c_str());
  if(send(socket_num,message.c_str(),stringLen,0)!=stringLen)
    {
      perror("send() failed");
      exit(1);
    }
}

void messagehandle::req_file_clnt()
{
  database mydb;
  string file = parse.get_str_between("[","]");

  cout << "File request for " << file << " from " << ip_address << endl;

  if( file == "" )
    {
      string message = "Error: no file given";
      int stringLen = strlen(message.c_str());
      if( send(socket_num,message.c_str(), stringLen,0) != stringLen )
	{
	  perror("send() failed in function req_file()" );
	  exit(1);
	}
      return;
    }

  mydb.clearBuffer();
  int num_ips = mydb.get_server_ip_list();

  int sock;			/*Socket discriptor*/
  struct sockaddr_in echoServAddr;/*the adress of the server socket*/
  string message;
  string iplist;
  int stringLen;
  int bytesRcv;
  char echoBuffer[RCVBUFFERSIZE];

  string send_to_client = search_server_for_file(file);
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
	  message = "req_file_serv~["+file+"]";
	  stringLen = strlen(message.c_str());
	  if( send(sock,message.c_str(), stringLen,0) != stringLen )
	    {
	      perror("send() failed in function req_file()" );
	      exit(1);
	    }
	  iplist = "";
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
	  
	  send_to_client = merge_ips( send_to_client, iplist );
	}
      
      close(sock);      
    }

  send_to_client += "<done>";
  stringLen = strlen(send_to_client.c_str());
  if( send(socket_num,send_to_client.c_str(), stringLen,0) != stringLen )
    {
      perror("send() failed in function req_file()" );
      exit(1);
    }
}

void messagehandle::req_file_serv()
{
  string file = parse.get_str_between("[","]");
  cout << "File request for " << file << " from " << ip_address << endl;
  if( file == "" )
    {
      string message = "Error: no file given";
      int stringLen = strlen(message.c_str());
      if( send(socket_num,message.c_str(), stringLen,0) != stringLen )
	{
	  perror("send() failed in function req_file()" );
	  exit(1);
	}
      return;
    }
  string message = search_server_for_file(file) + "<done>";
  
  int stringLen = strlen(message.c_str());
  if( send(socket_num,message.c_str(), stringLen,0) != stringLen )
    {
      perror("send() failed in function req_file()" );
      exit(1);
    }
}

string messagehandle::search_server_for_file(string filename)
{
  string ret = "";
  database mydb;

  mydb.clearBuffer();
  int num_files = mydb.search_file(filename);
  stringstream out;
  out << num_files;

  ret = "[" + out.str() + "]";
  for( int i = 0; i < num_files; i++ )
    {
      ret += "<" + mydb.getBuffer(i) + ">";
    }

  return ret;
}
