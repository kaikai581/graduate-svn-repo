#ifndef RPCCALIBTABLEDEF_HPP
#define RPCCALIBTABLEDEF_HPP


//#include <string>
//class Database;
#include "Database/Database.hpp"
#include "DataModel/rpcnamespace.hpp"
class DatasetHallData;


class RpcCalibDB : public Database
{
public:
  /// member variables
  
  /// table name for cluste size cut
  std::string tableName;
  /// table name for cluste size cut AND number of tracks cut
  std::string ntTableName;

  
  /// member functions
  RpcCalibDB(DatasetHallData&);
  ~RpcCalibDB();
  
  
  void createTable(std::string);
  void fillDB(DatasetHallData&);

private:
  /// member functions
  /// fill tables with cluster size and cluster size + number of tracks cut
  void insertRow2Table(rpc::moduleId, DatasetHallData&, DatasetModuleData&, std::string);
  
  /// Site is encoded in binary. Here convert it into human enumeration.
  unsigned int convertSite(unsigned int);
  /// pad with zeros to conform the SQL date format
  std::string padZeros(int, unsigned int);
};



#endif
