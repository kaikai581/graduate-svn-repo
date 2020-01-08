#include <cmath>
#include <iostream>
#include <sstream>
#include "Database/RpcCalibDB.hpp"
#include "DataModel/DatasetHallData.hpp"
#include "structopt.hpp"
#include "TTimeStamp.h"


using namespace std;



RpcCalibDB::RpcCalibDB(DatasetHallData& dsd)
{
  
  /// assign table names
  stringstream tblName;
  tblName << "cs" << dsd.clCutSize;
  tableName = tblName.str();
  tblName.str("");
  tblName << "ntcs" << dsd.clCutSize;
  ntTableName = tblName.str();
  
  
  /// open database connection
  option_t opt;
  stringstream dbName;
  dbName << opt.dbOutputPath << "/RpcCalib.db";
  open(dbName.str().c_str());
  
  /// create a table with a table name indicating the size of cluster size cut
  createTable(tableName);
  
  /// create the table for data with cluster size as well as number of
  /// tracks cut.
  createTable(ntTableName);
  
  /// fill database tables
  fillDB(dsd);
}




RpcCalibDB::~RpcCalibDB()
{
  close();
}





unsigned int RpcCalibDB::convertSite(unsigned int site)
{
  unsigned int hall = 1;
  
  while(!(site%2))
  {
    site /= 2;
    hall++;
  }
  
  return hall;
}






void RpcCalibDB::createTable(string tname)
{
  stringstream tableDef;
  tableDef << "CREATE TABLE IF NOT EXISTS " << tname;
  tableDef << "(";
  tableDef << "calib_data_id INTEGER PRIMARY KEY NOT NULL,";
  tableDef << "date          TEXT,";
  tableDef << "run_num       INTEGER,";
  tableDef << "file_num      INTEGER,";
  tableDef << "site          INTEGER,";
  tableDef << "row           INTEGER,";
  tableDef << "col           INTEGER,";
  tableDef << "rate          REAL,";
  tableDef << "rate_err      REAL,";
  tableDef << "eff           REAL,";
  tableDef << "eff_err       REAL,";
  tableDef << "UNIQUE (run_num,file_num,site,row,col)";
  tableDef << ");";

  query(tableDef.str().c_str());
}





void RpcCalibDB::fillDB(DatasetHallData& dsd)
{
  
  /// check if write to database flag is on
  option_t opt;
  if(!opt.dbFlag) return;
  
  unsigned int nModules = 0;
  
  
  cout << "Writing into table " << tableName;
  cout << ". Please be patient." << endl;
  for(rpc::dsModules::iterator it = dsd.modules.begin(); it != dsd.modules.end(); it++)
  {
    insertRow2Table(it->first, dsd, it->second, tableName);
    cout << ++nModules << " modules are done." << endl;
  }
  
  
  nModules = 0;
  cout << "Writing into table " << ntTableName;
  cout << ". Please be patient." << endl;
  for(rpc::dsModules::iterator it = dsd.modulesNT.begin(); it != dsd.modulesNT.end(); it++)
  {
    insertRow2Table(it->first, dsd, it->second, ntTableName);
    cout << ++nModules << " modules are done." << endl;
  }
}





void RpcCalibDB::insertRow2Table(rpc::moduleId mId, DatasetHallData& dhd,
     DatasetModuleData& dmd, string tname)
{
  unsigned int tlast = dmd.triggerTimeSec.size() - 1;
  
  TTimeStamp ts(dmd.triggerTimeSec[tlast], dmd.triggerTimeNanoSec[tlast]);
  unsigned int ymd = ts.GetDate();

  
  stringstream qry;
  qry << "INSERT OR REPLACE INTO " << tname;
  qry << "(date, run_num, file_num, site, row, col, rate, rate_err, eff, eff_err) VALUES(";
  qry << "'" << padZeros(4, ymd/10000) << "-";
  qry << padZeros(2, ymd%10000/100) << "-";
  qry << padZeros(2, ymd%100) << "',";
  qry << dhd.runNumber << "," << dhd.fileNumber << ",";
  qry << convertSite(dhd.site) << ",";
  qry << mId.first << ",";
  qry << mId.second << ",";
  qry << dmd.triggerRateCS.first << ",";
  qry << dmd.triggerRateCS.second << ",";
  qry << dmd.effCS[0].first << ",";
  qry << dmd.effCS[0].second;
  qry << ");";
  
  query(qry.str().c_str());
}









string RpcCalibDB::padZeros(int totdigits, unsigned int num)
{
  int ndigits = (int)log10(num) + 1;
  
  stringstream result;
  
  for(int i = ndigits; i < totdigits; i++) result << "0";
  result << num;
  
  return result.str();
}
