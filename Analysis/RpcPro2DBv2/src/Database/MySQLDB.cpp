#include "Database/MySQLDB.hpp"


using namespace std;


MySQLDB::MySQLDB()
{
  dbhost = string("tcp://heplinux9.phys.uh.edu:3306");
  user = string("root");
  password = string("aaaa5816");
  database = string("rpc_calibration");
}





void MySQLDB::connect()
{
  try
  {
    driver = get_driver_instance();
    
    con = driver -> connect(dbhost, user, password);
    
    con -> setAutoCommit(0);
    
    con -> setSchema(database);
  }
  catch (sql::SQLException &e)
  {
		cout << "ERROR: SQLException in " << __FILE__;
		cout << " (" << __func__<< ") on line " << __LINE__ << endl;
		cout << "ERROR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << ")" << endl;

		if (e.getErrorCode() == 1047) {
			/*
			Error: 1047 SQLSTATE: 08S01 (ER_UNKNOWN_COM_ERROR)
			Message: Unknown command
			*/
			cout << "\nYour server does not seem to support Prepared Statements at all. ";
			cout << "Perhaps MYSQL < 4.1?" << endl;
		}

		return;
	}
  catch (std::runtime_error &e)
  {
		cout << "ERROR: runtime_error in " << __FILE__;
		cout << " (" << __func__ << ") on line " << __LINE__ << endl;
		cout << "ERROR: " << e.what() << endl;

		return;
	}
}
