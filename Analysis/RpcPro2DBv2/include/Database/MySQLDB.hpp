#ifndef MYSQLDB_HPP
#define MYSQLDB_HPP


#include <string>

#include "cppconn/connection.h"
#include "cppconn/driver.h"
#include "cppconn/exception.h"
#include "cppconn/metadata.h"
#include "cppconn/prepared_statement.h"
#include "cppconn/resultset.h"
#include "cppconn/resultset_metadata.h"
#include "cppconn/statement.h"
#include "cppconn/warning.h"

class MySQLDB
{
public:
  std::string dbhost;
  std::string user;
  std::string password;
  std::string database;
  
  
  sql::Driver *driver;
  sql::Connection *con;
	sql::Statement *stmt;
	sql::ResultSet *res;
	sql::PreparedStatement *prep_stmt;
	sql::Savepoint *savept;
  
  
  
  
  MySQLDB();
  void setDbhost(std::string host) {dbhost = host;};
  void setUser(std::string u) {user = u;};
  void setPassword(std::string pw) {password = pw;};
  void setDatabase(std::string db) {database = db;};
  
  void connect();
};




#endif
