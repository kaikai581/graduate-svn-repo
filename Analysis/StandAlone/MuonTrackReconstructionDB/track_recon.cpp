/*
 * This program is to reconstruct muon tracks of the muons in the MySQL database
 * filled in with Event2DB_v04 NuWa Algorithm.
 * 
 * Shih-Kai Oct 2013
*/


#include <armadillo>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include "cppconn/resultset.h"
#include "cppconn/statement.h"
#include <iostream>
#include <string>
#include <tclap/CmdLine.h>

#define DBHOST "tcp://heplinux3.phys.uh.edu:3306"
#define USER "sklin"
#define DATABASE "EventDisplayDB_v043"


using namespace arma;
using namespace sql;
using namespace std;


pair<vec,vec> fitTrack(multimap<int,vec>&, vec&);


int main(int argc, char** argv)
{
  
  /// MySQL connection setup variables
  Driver* driver;
  Connection* con;
  Statement *stmt;
  ResultSet *res;
  
  /// container variables
  vector<int> vMuId;
  map<int, int> muHall;
  map<int, vec> owsOrigins;
  stringstream qrymu;
  
  
  /// parsing command line arguments
  TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
  
  TCLAP::ValueArg<string> pwArg("p", "password", "Database password", true, "input", "string");
  
  cmd.add(pwArg);
  
  cmd.parse(argc, argv);
  
  
  const string url(DBHOST);
  const string user(USER);
  const string password(pwArg.getValue());
  const string database(DATABASE);
  
  
  /// connect to database
  try
  {
    driver = get_driver_instance();
    
    /* create a database connection using the Driver */
    con = driver -> connect(url, user, password);
    
    /* turn off the autocommit */
    con -> setAutoCommit(0);
    
    cout << "\nDatabase connection\'s autocommit mode = " << con -> getAutoCommit() << endl;
    
    /* select appropriate database schema */
    con -> setSchema(database);
    
    cout << "Successfully connected to the database" << endl;
    
    
    /// retrieve all muon id in the muon table
    qrymu << "SELECT * from muon;";
    stmt = con->createStatement();
    res = stmt->executeQuery(qrymu.str());
    while (res->next())
    {
      vMuId.push_back(res->getInt("muonId"));
      muHall[res->getInt("muonId")] = res->getInt("hallId");
    }
    delete stmt;
    delete res;
    cout << vMuId.size() << " muon IDs retrieved" << endl;
    
    
    /// OWS origin of each hall is needed to calculate the ends of the track
    qrymu.str("");
    qrymu << "SELECT * FROM detector";
    qrymu << " WHERE detectorId = 6";
    stmt = con->createStatement();
    res = stmt->executeQuery(qrymu.str());
    while(res->next())
    {
      stringstream strcoord;
      strcoord << res->getDouble("originX") << " ";
      strcoord << res->getDouble("originY") << " ";
      strcoord << res->getDouble("originZ");
      vec org(strcoord.str());
      owsOrigins[res->getInt("hallId")] = org;
    }
    
    
    /// loop through each muon
    for(unsigned int i = 0; i < vMuId.size(); i++)
    {
      
      multimap<int,vec> recPts;
      
      /// store AD points
      qrymu.str("");
      qrymu << "SELECT * FROM muonTriggerConstituent mtc";
      qrymu << " INNER JOIN detectorTrigger dt ON mtc.detectorTriggerId = dt.detectorTriggerId";
      qrymu << " INNER JOIN recAdSimple ra ON mtc.detectorTriggerId = ra.detectorTriggerId";
      qrymu << " WHERE mtc.muonId = " << vMuId[i];
      stmt = con->createStatement();
      res = stmt->executeQuery(qrymu.str());

      /// store the coordinate
      while(res->next())
      {
        stringstream strcoord;
        strcoord << res->getDouble("x") << " ";
        strcoord << res->getDouble("y") << " ";
        strcoord << res->getDouble("z");
        vec p(strcoord.str());
        recPts.insert(pair<int,vec>(res->getInt("detectorId"),p));
      }
      
      delete stmt;
      delete res;
      
      /// store WP points
      qrymu.str("");
      qrymu << "SELECT * FROM muonTriggerConstituent mtc";
      qrymu << " INNER JOIN detectorTrigger dt ON mtc.detectorTriggerId = dt.detectorTriggerId";
      qrymu << " INNER JOIN recPoolSimple rp ON mtc.detectorTriggerId = rp.detectorTriggerId";
      qrymu << " WHERE mtc.muonId = " << vMuId[i];
      stmt = con->createStatement();
      res = stmt->executeQuery(qrymu.str());
      
      /// store the coordinate
      while(res->next())
      {
        stringstream strcoord;
        strcoord << res->getDouble("x") << " ";
        strcoord << res->getDouble("y") << " ";
        strcoord << res->getDouble("z");
        vec p(strcoord.str());
        recPts.insert(pair<int,vec>(res->getInt("detectorId"),p));
      }
      
      delete stmt;
      delete res;
      
      /// store RPC points
      qrymu.str("");
      qrymu << "SELECT * FROM muonTriggerConstituent mtc";
      qrymu << " INNER JOIN detectorTrigger dt ON mtc.detectorTriggerId = dt.detectorTriggerId";
      qrymu << " INNER JOIN recRpcSimple rr ON mtc.detectorTriggerId = rr.detectorTriggerId";
      qrymu << " WHERE mtc.muonId = " << vMuId[i];
      stmt = con->createStatement();
      res = stmt->executeQuery(qrymu.str());
      
      /// store the coordinate
      while(res->next())
      {
        stringstream strcoord;
        strcoord << res->getDouble("x") << " ";
        strcoord << res->getDouble("y") << " ";
        strcoord << res->getDouble("z");
        vec p(strcoord.str());
        int detId = res->getInt("detectorId");
        /// set RPC telescope's detector ID as 8
        if(res->getBoolean("isTelescope")) detId++;
        recPts.insert(pair<int,vec>(detId,p));
      }
      
      delete stmt;
      delete res;
      
      
      
      /// determine whether to reconstruct the track
      bool multiRpcP = (recPts.count(7) > 1);
      int np = 0;
      for(multimap<int,vec>::iterator it = recPts.begin(); it != recPts.end(); it = recPts.upper_bound(it->first))
        np++;
      
      
      //cout << vMuId[i] << " " << np << endl;
      if( np >= 2 && !multiRpcP )
      {
        pair<vec,vec> fittedtrack = fitTrack(recPts, owsOrigins[muHall[vMuId[i]]]);
      
        /// insert the track
        qrymu.str("");
        qrymu << "INSERT IGNORE INTO recTrackFitAll";
        qrymu << " SET muonId = " << vMuId[i];
        qrymu << ", x1 = " << fittedtrack.first(0);
        qrymu << ", y1 = " << fittedtrack.first(1);
        qrymu << ", z1 = " << fittedtrack.first(2);
        qrymu << ", x2 = " << fittedtrack.second(0);
        qrymu << ", y2 = " << fittedtrack.second(1);
        qrymu << ", z2 = " << fittedtrack.second(2);
        qrymu << ";";
        
        stmt = con->createStatement();
        stmt->executeUpdate(qrymu.str());
        
        delete stmt;
      }
      
    }/// end of muon loop
    
    con->commit();
    con->close();
    delete con;
    
  }
  catch (SQLException &e)
  {
    cout << "ERROR: SQLException in " << __FILE__;
    cout << " (" << __func__<< ") on line " << __LINE__ << endl;
    cout << "ERROR: " << e.what();
    cout << " (MySQL error code: " << e.getErrorCode();
    cout << ", SQLState: " << e.getSQLState() << ")" << endl;
    
    if (e.getErrorCode() == 1047)
    {
      /*
       * Error: 1047 SQLSTATE: 08S01 (ER_UNKNOWN_COM_ERROR)
       * Message: Unknown command
      */
      
      cout << "\nYour server does not seem to support Prepared Statements at all. ";
      cout << "Perhaps MYSQL < 4.1?" << endl;
    }
    
    return EXIT_FAILURE;
  }
  catch (runtime_error &e)
  {
    cout << "ERROR: runtime_error in " << __FILE__;
    cout << " (" << __func__ << ") on line " << __LINE__ << endl;
    cout << "ERROR: " << e.what() << endl;
    
    return EXIT_FAILURE;
  }
  
  
  return 0;
}



pair<vec,vec> fitTrack(multimap<int,vec>& pts, vec& pO)
{
  
  /// the virtical distance from the track ends to the center
  const double dz = 8400.;
  
  /// calculate the centroid of the point cloud
  vec pbar("0. 0. 0.");
  int np = 0;
  multimap<int,vec>::iterator it = pts.begin();
  for(; it != pts.end(); it++)
  {
    np++;
    pbar += it->second;
  }
  pbar /= (double)np;
  
  /// start forming the chi-squared matrix
  it = pts.begin();
  mat mchi2(3,3);
  mchi2.zeros();
  for(; it != pts.end(); it++)
  {
    vec d = it->second - pbar;
    mat idm;
    idm.eye(3,3);
    mchi2 += dot(d,d)*idm - d*d.t();
  }
  //cout << mchi2 << endl;
  
  vec eigval;
  mat eigvec;
  eig_sym(eigval, eigvec, mchi2);
  /// always make the direction vector point upward
  vec vdir = eigvec.col(0);
  if(vdir(2) < 0) vdir *= -1;
  
  double ztop = pO(2) + dz;
  double zbot = pO(2) - dz;
  
  vec vtop, vbot;
  
  vtop = pbar + (ztop-pbar(2))/vdir(2)*vdir;
  vbot = pbar + (zbot-pbar(2))/vdir(2)*vdir;
  
  return pair<vec,vec>(vtop,vbot);
}
