#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "TFile.h"
#include "TH1F.h"
#include "TF1.h"
#include "structopt.hpp"
#include "utilities.hpp"


#define DTTRIGGERBINSIZE 0.05 // bin size of 4-fold dt histogram in second


using namespace std;

double getErrFromPaper(double, double);
void getRateErr(string, string, int, int, int, vector<double>&, double&,
                double&);
void getCorrectedRateErr(double, double, double, double, double&, double&);
void rawRate2File(option_t, double, double);



void getEffByQuotient(string ifp, string ofp, int hall, int row, int col,
                      bool dbgFlag, unsigned int N4, unsigned int N3[],
                      vector<double>& triggerDt)
{
  /// print 3-fold and 4-fold trigger numbers
  cout << endl;
  for(int i = 0; i < 80; i++)
    cout << "-";
  cout << "***************************results of quotient method";
  cout << "***************************" << endl;
  cout << "number of triggers:" << endl;
  cout << "4-fold: " << N4 << endl;
  cout << "3-fold with layer i absent:";
  for(int j = 0; j < 4; j++)
    cout << " " << N3[j];
  cout << endl << endl;
  
  
  double e[4] = {0.}, ee[4] = {0.};
  unsigned int N3iN4[4];
  for(int i = 0; i < 4; i++)
  {
    N3iN4[i] = N3[i] + N4;
    if(!N3iN4[i])
    {
      cerr << "sum of number of 4-fold triggers and number of 3-fold triggers ";
      cerr << "with layer " << i+1 << " absent is zero" << endl;
      continue;
    }
    e[i] = (double)N4/(double)N3iN4[i];
    ee[i] = getErrFromPaper((double)N4, (double)N3iN4[i]);
    
    cout << "efficiency and error for layer " << i+1 << ": ";
    cout << e[i] << " " << ee[i] << endl;
  }
  
  
  /// calculate module efficiency and error
  double mEff = 0., mErr = 0.;
  getModuleEffErr(e, ee, mEff, mErr);
  cout << endl;
  cout << "module efficiency and error: " << endl;
  cout << mEff << " " << mErr << endl;
  
  /// calculate trigger rate and error
  double rate = 0., erate = 0.;
  getRateErr(ifp, ofp, hall, row, col, triggerDt, rate, erate);
  cout << "trigger rate and error: ";
  cout << rate << " " << erate << endl;
  
  /// calculate corrected rate and error
  double cRate, ecRate;
  getCorrectedRateErr(rate, erate, mEff, mErr, cRate, ecRate);
  cout << "corrected rate and error: ";
  cout << cRate << " " << ecRate << endl;


  /// output results to a file
  eff2File("ByQuotient", ifp, ofp, hall, row, col, mEff, mErr,
           cRate, ecRate, e, ee);

  option_t opt;
  opt.inputPath = ifp;
  opt.outputPath = ofp;
  opt.hall = hall;
  opt.row = row;
  opt.col = col;
  /// output raw rate to a file
  rawRate2File(opt, rate, erate);
}




double getErrFromPaper(double k, double n)
{
  return sqrt((k+1)/(n+2)*(k+2)/(n+3)-(k+1)*(k+1)/(n+2)/(n+2));
}




void getRateErr(string ifp, string ofp, int hall, int row, int col,
                vector<double>& triggerDt, double& rate, double& erate)
{
  
  string year = getYear(ifp);
  string date = getDate(ifp);
  
  TFile fof(Form("%s/dt_triggers.root", ofp.c_str()), "update");
  
  string hName = Form("dtTriggerEH%dR%dC%d_%s%s", hall, row, col,
                      year.c_str(), date.c_str());


  /// override if exists
  TObject *obj = (TObject*)fof.Get(hName.c_str());
  if(obj) fof.Delete(Form("%s;*", hName.c_str()));
  
  
  double maxDt = *max_element(triggerDt.begin(), triggerDt.end());
  
  TH1F hDtTrigger(hName.c_str(), hName.c_str(), (int)(maxDt/DTTRIGGERBINSIZE),
                  0, maxDt);
  
  
  for(unsigned int i = 0; i < triggerDt.size(); i++)
    hDtTrigger.Fill(triggerDt[i]);
  hDtTrigger.Write();
  fof.Close();
  
  TF1 ffit("ffit", "expo");
  hDtTrigger.Fit("ffit", "Q");
  rate = fabs(ffit.GetParameter(1));
  erate = ffit.GetParError(1);
}



void getCorrectedRateErr(double rate, double erate, double eff, double efferr,
                         double& crate, double& ecrate)
{
  if(eff == 0.)
  {
    cerr << "module efficiency with quotient method is 0." << endl;
    crate = 0.;
    ecrate = 0.;
  }
  else if(rate == 0.)
  {
    cerr << "trigger rate with fitting method is 0." << endl;
    crate = 0.;
    ecrate = 0.;
  }
  else
  {
    crate = rate/eff;
    double t1 = erate/rate;
    double t2 = efferr/eff;
    ecrate = crate*sqrt(t1*t1+t2*t2);
  }
}




void rawRate2File(option_t o, double r, double er)
{
  map<string, string> dataLine;
  string curDate = getYear(o.inputPath) + getDate(o.inputPath);
  string ofpn = Form("%s/RawRateEH%dR%dC%d.txt", o.outputPath.c_str(),
                     o.hall, o.row, o.col);
  if(fExists(ofpn.c_str()))
  {
    ifstream ifpn(ofpn.c_str());
    
    string dateThisLine;
    string restLine;
    while(ifpn >> dateThisLine)
    {
      getline(ifpn, restLine);
      if(dateThisLine == curDate) continue;
      else
        dataLine[dateThisLine] = restLine;
    }
    
    ifpn.close();
  }
  

  stringstream restData;
  restData << " " << r << " " << er;
  dataLine[curDate] = restData.str();
  
  
  ofstream ofStr(ofpn.c_str());
  for(map<string, string>::iterator it = dataLine.begin(); it != dataLine.end();
      it++)
    ofStr << it->first << it->second << endl;
  ofStr.close();
}
