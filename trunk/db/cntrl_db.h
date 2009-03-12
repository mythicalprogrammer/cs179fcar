#ifndef __CNTRL_DB_H__
#define __CNTRL_DB_H__

#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

class cntrl_db 
{
 public:
  void db_init();

  void insert_ip(string);
  void delete_server_IP(string);
  int get_ip_list();

  string getBuffer(int i)
  {return buffer[i];};

  void clearBuffer()
  {buffer.clear();};

 private:
  int RunCommand(const char *strCommand);

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
