#ifndef __MESSAGEHANDLE_H__
#define __MESSAGEHANDLE_H__

#include<string>

using namespace std;

class messagehandle{

public:
	messagehandle();
	messagehandle(string);
	~messagehandle();
	
	void parse_message(string);
	
private:
	string handle;
	string ip_address;
	//int sock;
	
	// Message recieve functions
	void add_server();
	
};
#endif
