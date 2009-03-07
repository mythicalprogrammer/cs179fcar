#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
WARNING NOTE
WARNING NOTE
WARNING NOTE

you need to link the "-sqlite3" object file

-lsqlite3

ex:
g++ database.cpp -lsqlite3

*/

using namespace std;

class database {
 public:
  database();
  /*
   * Function: void db_init()
   * Basically makes the CAR database
   * if the CAR database exist it'll try to make it and complain
   * it won't overwrite the existing database
   */
  void db_init();
  
  /*
   * Function: int RunCommand(const char *strCommand)
   * Runs a series of command that create CAR database
   */
  int RunCommand(const char *strCommand);
  
  /*
   * Function: void insert_ip(string sql)
   * Use this function to insert list of ips/clients/users
   */
  void insert_ip(string sql); // this insert to listIP table
  
  /*
   * Function: void insert_ip(string sql)
   * Use this function to insert all files and what user are sharing them
   */
  void insert_file(string file_sql, string ip_sql); //this insert to the listIP table
  
  
  /*
   * Function: int search_file(string fileName)
   * Search function, returns number of results
   * store the results/ip into a buffer or vector of strings                      
   */            
  int search_file(string fileName);
  
  //void searchHelper ();
  string getBuffer(int i){  return buffer[i]; };
  void clearBuffer(){ buffer.clear(); };    
  
  void deleteClientIP(string ip);
  
  void deleteServerIP(string ip);
  
  void insert_server_ip(string sql);
  
  int get_server_ip_list();
  
 private:
  string query;
  vector<string> buffer;    // a list of queries
  int iForkId, iStatus;
  int iNumProc, iChildiStatus, iDeadId;
  int iExitFlag,rc;
  sqlite3 *db;
  string dbname;
  const char* dbnameCstr;
  char *errmsg;
  char *zErrMsg;
  char **queryResult;       /* Results of the query */
  int pnRow;                /* Number of result rows written here */
  int pnColumn;             /* Number of result columns written here */
};
#endif
