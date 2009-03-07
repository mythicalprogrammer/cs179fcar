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
  void set_IPaddress(string);
  void parse_message();
	
private:
  int socket_num;
  string ip_address;
  strparse parse;
	
  // how to handle recieved messages
  void ping();
  void add_server();
  void add_client();
  void add_file();
  void add_file_list();
  void req_file_clnt();
  void req_file_serv();

  string search_server_for_file(string);
};
#endif
