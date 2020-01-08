#include <climits> // where PATH_MAX is defined
#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include <string>
#include "TROOT.h"

using namespace std;

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
    ofn = "rpctree." + ofn;
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


bool fExists(const char *filename)
{
  struct stat buffer;
  /// From stat.h manual:
  /// Upon successful completion, 0 shall be returned.
  /// Otherwise, -1 shall be returned and errno set to indicate the error.
  if (stat(filename, &buffer) == 0) return true;
  return false;
}


void globalSettings()
{
  char macroPath[PATH_MAX];
  getcwd(macroPath, PATH_MAX);
  gROOT->SetMacroPath(Form("%s/include", macroPath));
  gROOT->ProcessLine(".L vecdict.h+");
  gROOT->ProcessLine(".x $ROOTIOTESTROOT/share/load.C");
}



void noRpcList(string infpn)
{
  vector<string> vFileName;
  if(fExists("NoRpcFileList.txt"))
  {
    string thisFileName;
    ifstream fList("NoRpcFileList.txt");
    while(fList >> thisFileName)
      vFileName.push_back(thisFileName);
    fList.close();
    
    vector<string>::iterator itEnd = vFileName.end();
    if(find(vFileName.begin(), itEnd, infpn) == itEnd)
    {
      ofstream ofList;
      ofList.open("NoRpcFileList.txt", ios::out|ios::app);
      ofList << infpn << endl;
      ofList.close();
    }
  }
  else
  {
    ofstream ofList("NoRpcFileList.txt");
    ofList << infpn << endl;
    ofList.close();
  }
  return;
}
