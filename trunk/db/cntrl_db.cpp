#include "cntrl_db.h"

void cntrl_db::db_init()
{
  rc = 0;
  dbname ="cntrl.db";
  iNumProc = 0, iChildiStatus = 0, iStatus = 0, iDeadId = 0, iExitFlag = 0;

  dbnameCstr = const_cast<char *>(dbname.c_str()); 

  rc = sqlite3_open(dbnameCstr, &db);

  //create the cardb database and import the schema
  iStatus = RunCommand("sqlite3 cntrl.db < cntrl_schema.dat");

  if (!iStatus)
    iNumProc++;
        
  // Wait till the commands complete
  while (iNumProc && !iExitFlag)
    {
      iDeadId = waitpid(-1, &iChildiStatus, WNOHANG);
      if (iDeadId < 0)
        {
          // Wait id error - exit the loop
          iExitFlag = 1;
        }
      else if (iDeadId > 0)
        {
          iNumProc--;
          // You can check the process exit iStatus here - its in the
          // iChildiStatus variable
        }
      else  // iDeadId == 0, no processes died
        {
          sleep(3);     // give them time to die
        }
      
    } //end while (iNumProc && !iExitFlag) 
  /* Jan Koci code portion ends here.*/
  
}

int cntrl_db::RunCommand(const char *strCommand)
{
  /* Function by...
   * Author: Jan Koci 
   * Modified by: Quoc Anh Doan
   */

  iForkId = vfork();
  if (iForkId == 0)     // This is the child 
    {
      iStatus = execl("/bin/sh","sh","-c", strCommand, (char*) NULL);
      exit(iStatus);    // We must exit here, 
      // or we will have multiple
      // mainlines running...  
    }
  else if (iForkId > 0) // Parent, no error
    {
      iStatus = 0;
    }
  else  // Parent, with error (iForkId == -1)
    {
      iStatus = -1;
    }
  return(iStatus);

}

void cntrl_db::insert_ip(string sql) //this insert to the listIP table
{
  string temp = "insert into IPaddresses values(\"" + sql;
  temp = temp + "\");"; 

  const char* sqlCstr = const_cast<char *>(temp.c_str()); 
  int prepare;

  prepare = sqlite3_exec(db, sqlCstr, NULL, NULL, NULL );
}

int cntrl_db::get_ip_list()
{

  dbnameCstr = const_cast<char *>(dbname.c_str()); 

  if( rc!=SQLITE_OK )
    {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    } 

  string sql = "SELECT * FROM IPaddresses;";
  const char* sqlCstr = const_cast<char *>(sql.c_str());
  int prepare;

  prepare = sqlite3_get_table( db, sqlCstr, &queryResult, 
			       &pnRow, &pnColumn, &errmsg); 
  sqlite3_free(errmsg);

  for(int i = 1; i <= pnRow; ++i)
    {
      buffer.push_back(queryResult[i]);
    }

  int result = pnRow;
  sqlite3_free_table(queryResult);
  return result; 
}

void cntrl_db::delete_server_IP(string ip)
{
  string temp = "DELETE FROM IPaddresses WHERE address='" + ip;
  temp = temp + "';"; 

  const char* sqlCstr = const_cast<char *>(temp.c_str());
  int prepare;
  prepare = sqlite3_exec(db, sqlCstr, NULL, NULL, NULL );
}
