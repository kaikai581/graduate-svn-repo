#include <climits> // where PATH_MAX is defined
#include <cmath>
#include <dirent.h>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include "TROOT.h"


using namespace std;


string trimTrailingSlash(string& instr)
{
  while(instr.rfind("/") == instr.length()-1)
    instr = instr.substr(0, instr.length()-1);
  return instr;
}


bool isRootFile(string instr)
{
  if(instr.substr(instr.size()-5,5)==".root")
    return true;
  return false;
}



bool checkInputValidity(string ifp, int hall)
{
/*
 * This part is for handling path
 */
  if(!isRootFile(ifp))
    gROOT->ProcessLine(Form(".! ls %s/*EH%d*.root>infiles", ifp.c_str(), hall));
  else
    gROOT->ProcessLine(Form(".! ls %s>infiles", ifp.c_str()));
  ifstream inflist("infiles");
  
  bool res = true;
  string tmpstr;
  if(!(inflist >> tmpstr)) res = false;
  inflist.close();
  gROOT->ProcessLine(".! rm -rf infiles");
  
  return res;
}



void loadDictionary()
{
  char macroPath[PATH_MAX];
  getcwd(macroPath, PATH_MAX);
  gROOT->SetMacroPath(Form("%s/include", macroPath));
  gROOT->ProcessLine(".L vecdict.h+");
}




string getYear(string ifStr)
{
  string year;
  
  int nRemoval;
  if(isRootFile(ifStr)) nRemoval = 3;
  else nRemoval = 2;
    
  for(int i = 0; i < nRemoval; i++)
    ifStr = ifStr.substr(0, ifStr.rfind("/"));

  int lastSlashPosition = ifStr.rfind("/");

  if(lastSlashPosition > 4)
    year = ifStr.substr(lastSlashPosition-4, 4);
  return (year.length()) ? year : "";
}



string getDate(string ifStr)
{
  string date;
  
  if(isRootFile(ifStr))
  {
    int lastSlashPosition = ifStr.rfind("/");
    if(lastSlashPosition > 4)
      date = ifStr.substr(lastSlashPosition-4, 4);
  }
  else
    date = ifStr.substr(ifStr.length()-4,4);
  
  return (date.length()) ? date : "";
}




bool fExists(const char *filename)
{
  struct stat buffer;
  /// From stat.h manual:
  /// Upon successful completion, 0 shall be returned.
  /// Otherwise, -1 shall be returned and errno set to indicate the error.
  if (stat(filename, &buffer) == 0) return true;
  return false;
}



/// calculate module efficiency
void getModuleEffErr(double lEff[], double lErr[], double& mEff, double& mErr)
{
  double fourProduct = 1.;
  for(int l = 0; l < 4; l++)
  {
    double threeProduct = 1-lEff[l];
    for(int p = 1; p < 4; p++)
      threeProduct *= lEff[(l+p)%4];
    mEff += threeProduct;
    fourProduct *= lEff[l];
  }
  mEff += fourProduct;



  /// calculate error according to layer errors
  double D1f, D2f, D3f, D4f;
  double x1, x2, x3, x4;
  double dx1, dx2, dx3, dx4;
  x1 = lEff[0];
  dx1 = lErr[0];
  x2 = lEff[1];
  dx2 = lErr[1];
  x3 = lEff[2];
  dx3 = lErr[2];
  x4 = lEff[3];
  dx4 = lErr[3];
  D1f = x3*x4+x2*x4+x2*x3-3*x2*x3*x4;
  D2f = x1*x3+x3*x4+x1*x4-3*x1*x3*x4;
  D3f = x1*x2+x2*x4+x1*x4-3*x1*x2*x4;
  D4f = x1*x2+x2*x3+x1*x3-3*x1*x2*x3;
  D1f *= dx1;
  D2f *= dx2;
  D3f *= dx3;
  D4f *= dx4;
  mErr = sqrt(D1f*D1f+D2f*D2f+D3f*D3f+D4f*D4f);
  /// end of error calculation
}




void eff2File(const char* method, string ifp, string ofp, int hall, int row,
              int col, double moduleEff, double moduleEffErr, double realRate,
              double eRealRate, double layerEff[], double layerEffErr[])
{
  map<string, string> dataLine;
  string curDate = getYear(ifp) + getDate(ifp);
  string ofpn = Form("%s/eff%sEH%dR%dC%d.txt", ofp.c_str(),
                     method, hall, row, col);
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
  restData << " " << moduleEff << " " << moduleEffErr << " ";
  restData << realRate << " " << eRealRate;
  for(int i = 0; i < 4; i++)
    restData << " " << layerEff[i] << " " << layerEffErr[i];
  dataLine[curDate] = restData.str();
  
  
  ofstream ofStr(ofpn.c_str());
  for(map<string, string>::iterator it = dataLine.begin(); it != dataLine.end();
      it++)
    ofStr << it->first << it->second << endl;
  ofStr.close();
}




void getInputFilePathnames(vector<string>& vifpn, string cmdPath)
{
  gROOT->ProcessLine(Form(".! ls %s/*.root>filelist", cmdPath.c_str()));
  
  ifstream ffl("filelist");
  string tmpstr;
  while(ffl >> tmpstr) vifpn.push_back(tmpstr);
  ffl.close();
  
  gROOT->ProcessLine(".! rm -rf filelist");
}



string getOutputPathName(string ifpn, string ofp)
{
  string ofpn = "", ofn;
  
  /// check if output filename is specified
  if(ofp.length()>5 && ofp.substr(ofp.length()-5, 5) == ".root")
  {
    if(ofp.rfind('/') == string::npos)
    {
      ofn = ofp;
      ofp = "./";
    }
    else
    {
      ofn = ofp.substr(ofp.rfind('/')+1, ofp.length()-1-ofp.rfind('/'));
      ofp = ofp.substr(0, ofp.rfind('/'));
    }
  }
  /// if output file name is not specified
  else
  {
    ofn = ifpn;
    if(ifpn.rfind('/') != string::npos)
      ofn = ofn.substr(ofn.rfind('/')+1, ofn.length()-1-ofn.rfind('/'));
    
    if(ofp.length() == 0)
      ofp = "./";
      
    
    ofn = ofn.substr(ofn.find('.')+1, ofn.length()-1-ofn.find('.'));
    ofn = "enrichedrpctree." + ofn;
  }
  
  if(ofp.rfind('/') != ofp.length()-1)
    ofp += "/";
  ofpn = ofp + ofn;
  
  /// check if the specified path exists
  struct stat st;
  if(stat(ofp.c_str(), &st) != 0)
    ofpn = "";
  
  return ofpn;
}




string getRunNumber(string ifpn)
{
  for(int i = 0; i < 2; i++)
  {
    if(ifpn.find(".") == string::npos)
      return string("");
    ifpn = ifpn.substr(ifpn.find(".")+1, ifpn.length()-ifpn.find("."));
  }
  return ifpn.substr(0, 7);
}





string getFileNumber(string ifpn)
{
  if(ifpn.find("_") == string::npos)
    return string("");
  return ifpn.substr(ifpn.find("_")+1, 4);
}
