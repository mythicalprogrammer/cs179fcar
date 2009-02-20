#ifndef __MESSAGEHANDLE_H__
#define __MESSAGEHANDLE_H__

#include <string>
#include "strparse.h"

using namespace std;

class messagehandle{

public:
	messagehandle();
	messagehandle(string);
	~messagehandle();
	
	void set_socket(int);
	void parse_message();
	
private:
	int socket;
	strparse parse;
	
	// how to handle recieved messages
	void ping();
	void add_server();
	void add_client();
	void add_file();
	void add_file_list();

};
#endif
