#include "database.h"
 
database::database()
{
  rc = 0;
  dbname="cardb";
  iNumProc = 0, iChildiStatus = 0, iStatus = 0, iDeadId = 0, iExitFlag = 0;
  dbnameCstr = const_cast<char *>(dbname.c_str()); //dbname has to be in const char*
  rc = sqlite3_open(dbnameCstr, &db);
}
void database::db_init()
{
  rc = 0;
  dbname ="cardb";
  iNumProc = 0, iChildiStatus = 0, iStatus = 0, iDeadId = 0, iExitFlag = 0;

  dbnameCstr = const_cast<char *>(dbname.c_str()); //dbname has to be in const char*

  rc = sqlite3_open(dbnameCstr, &db);

  /* 
   * Author: Jan Koci 
   * Modified by: Quoc Anh Doan
   */
  
  //create the cardb database and import the schema
  iStatus = RunCommand("sqlite3 cardb < schema");
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

int database::RunCommand(const char *strCommand)
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

void database::insert_ip(string sql) //this insert to the listIP table
{
  /*
    int sqlite3_exec(
    sqlite3*,                                  / An open database /
    const char *sql,                           / SQL to be evaluated /
    int (*callback)(void*,int,char**,char**),  / Callback function /
    void *,                                    / 1st argument to callback /
    char **errmsg                              / Error msg written here /
    );
  */
  //dbname ="cardb";
 // rc = sqlite3_open(dbnameCstr, &db);


  string temp = "insert into listIP values(\"" + sql;
  temp = temp + "\");"; 

  const char* sqlCstr = const_cast<char *>(temp.c_str()); //has to be in const char*
  int prepare;

  prepare = sqlite3_exec(db, sqlCstr, NULL, NULL, NULL );
}

void database::insert_file(string file_sql, string ip_sql) //this insert to the listIP table
{
  /*
    int sqlite3_exec(
    sqlite3*,                                  / An open database /
    const char *sql,                           / SQL to be evaluated /
    int (*callback)(void*,int,char**,char**),  / Callback function /
    void *,                                    / 1st argument to callback /
    char **errmsg                              / Error msg written here /
    );
  */
  //dbname ="cardb";
  //rc = sqlite3_open(dbnameCstr, &db);

  string temp = "insert into files values(\"" + file_sql + "\",\"" + ip_sql + "\");";
  const char* sqlCstr = const_cast<char *>(temp.c_str()); //has to be in const char*
  int prepare;

  prepare = sqlite3_exec(db, sqlCstr, NULL, NULL, NULL );

}

int database::search_file(string fileName)
{

  /*
    int sqlite3_get_table(
    sqlite3 *db,          / An open database /
    const char *zSql,     / SQL to be evaluated /
    char ***pazResult,    / Results of the query /
    int *pnRow,           / Number of result rows written here /
    int *pnColumn,        / Number of result columns written here /
    char **pzErrmsg       / Error msg written here /
    );
  */
  
  //rc = 0;
  //dbname ="cardb";
  dbnameCstr = const_cast<char *>(dbname.c_str()); //dbname has to be in const char*
  //rc = sqlite3_open(dbnameCstr, &db);


  if( rc!=SQLITE_OK )
    {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    } 

  string sql = "select clientip from files where names='"+ fileName + "'";
  const char* sqlCstr = const_cast<char *>(sql.c_str()); //has to be in const char*
  int prepare;



  prepare = sqlite3_get_table( db, sqlCstr, &queryResult, &pnRow, &pnColumn, &errmsg); 
  sqlite3_free(errmsg);
  int i;

  for(i = 1; i <= pnRow; ++i)
    {
      buffer.push_back(queryResult[i]);
    }

  int result = pnRow;
  sqlite3_free_table(queryResult);
  return result; 
}

int database::get_server_ip_list()
{
  dbnameCstr = const_cast<char *>(dbname.c_str()); //dbname has to be in const char*
  if( rc!=SQLITE_OK )
    {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    } 

  string sql = "SELECT * FROM serverIP;";
  const char* sqlCstr = const_cast<char *>(sql.c_str()); //has to be in const char*
  int prepare;

  prepare = sqlite3_get_table( db, sqlCstr, &queryResult, &pnRow, &pnColumn, &errmsg); 
  sqlite3_free(errmsg);
  int i;

  for(i = 1; i <= pnRow; ++i)
    {
      buffer.push_back(queryResult[i]);
    }

  int result = pnRow;
  sqlite3_free_table(queryResult);

  return result; 
}

int database::get_client_ip_list()
{
  dbnameCstr = const_cast<char *>(dbname.c_str()); //dbname has to be in const char*
  if( rc!=SQLITE_OK )
    {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    } 

  string sql = "SELECT * FROM listIP;";
  const char* sqlCstr = const_cast<char *>(sql.c_str()); //has to be in const char*
  int prepare;

  prepare = sqlite3_get_table( db, sqlCstr, &queryResult, &pnRow, &pnColumn, &errmsg); 
  sqlite3_free(errmsg);
  int i;

  for(i = 1; i <= pnRow; ++i)
    {
      buffer.push_back(queryResult[i]);
    }

  int result = pnRow;
  sqlite3_free_table(queryResult);

  return result; 
}

/*
void db::searchHelper ()
{ 
  int i;

  for(i = 1; i <= pnRow; ++i)
    {
      buffer.push_back(queryResult[i]);
    }
  sqlite3_free_table(queryResult);
}
*/

void database::deleteClientIP(string ip)
{

//DELETE FROM pies WHERE flavour='Lemon Meringue'/

  string temp = "delete from listIP where clientip='" + ip;
  temp = temp + "';"; 

  const char* sqlCstr = const_cast<char *>(temp.c_str()); //has to be in const char*
  int prepare;
  prepare = sqlite3_exec(db, sqlCstr, NULL, NULL, NULL );
}

void database::insert_server_ip(string sql) //this insert to the listIP table
{
  /*
    int sqlite3_exec(
    sqlite3*,                                  / An open database /
    const char *sql,                           / SQL to be evaluated /
    int (*callback)(void*,int,char**,char**),  / Callback function /
    void *,                                    / 1st argument to callback /
    char **errmsg                              / Error msg written here /
    );
  */
  //dbname ="cardb";
 // rc = sqlite3_open(dbnameCstr, &db);


  string temp = "insert into serverIP values(\"" + sql;
  temp = temp + "\");"; 

  const char* sqlCstr = const_cast<char *>(temp.c_str()); //has to be in const char*
  int prepare;

  prepare = sqlite3_exec(db, sqlCstr, NULL, NULL, NULL );
}

void database::deleteServerIP(string ip)
{

//DELETE FROM pies WHERE flavour='Lemon Meringue'/

  string temp = "delete from serverIP where serverip='" + ip;
  temp = temp + "';"; 

  const char* sqlCstr = const_cast<char *>(temp.c_str()); //has to be in const char*
  int prepare;
  prepare = sqlite3_exec(db, sqlCstr, NULL, NULL, NULL );
}
