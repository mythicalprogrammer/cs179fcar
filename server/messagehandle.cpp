#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include "messagehandle.h"
#include "strparse.h"
#include <string>
#include <iostream>

using namespace std;

void to_lower(string& s)
{
	for( unsigned i = 0; i < s.length(); i++ )
		if( (int)s[i] > 64 && (int)s[i] < 91 )
			s[i] = (char)( (int)s[i] + 32 );
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
	socket = i;
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
	string message = "pong";
	int stringLen = strlen(message.c_str());
	
	if(send(socket,message.c_str(),stringLen,0)!=stringLen)
	{
		perror("send() failed");
		exit(1);
	}
}

void messagehandle::add_server()
{
	cout << "Adding server\n";
}
void messagehandle::add_client()
{
	cout << "Adding client\n";
}
void messagehandle::add_file()
{
	cout << "Adding file\n";
}
void messagehandle::add_file_list()
{
	cout << "Adding multiple files\n";
}
