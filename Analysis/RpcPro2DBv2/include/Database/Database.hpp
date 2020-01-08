#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <string>
#include <vector>
#include <sqlite3.h>

using namespace std;

class Database
{
public:
  Database();
	Database(char* filename);
	~Database();
	
	bool open(const char* filename);
	vector<vector<string> > query(const char* query);
	void close();
	
private:
	sqlite3 *database;
};

#endif

