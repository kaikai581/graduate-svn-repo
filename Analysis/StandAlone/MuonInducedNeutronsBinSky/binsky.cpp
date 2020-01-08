/*
 * To make a file list, one can use the following command:
 * $ find /global/project/projectdirs/dayabay/scratch/sklin/RootFile/MuonLookupTable/MuonInducedNeutron -type f|xargs ls > filelist.txt
 * A much faster command although unsorted is:
 * $ find /global/project/projectdirs/dayabay/scratch/sklin/RootFile/MuonLookupTable/MuonInducedNeutron -type f > filelist.txt
 * 
 * 
 * Fix phi direction to East, West, North and South.
 * Choose theta so that the projection of the cone to the RPC plane is fully
 * covered by RPC.
 * Choose muons within 15 degree half opening angle around the direction
 * and look at the neutron lateral distribution.
 * 
 * Nov 2013 by Shih-Kai.
 */


#include <stdlib.h>
#include <fstream>
#include "TChain.h"
#include "TFile.h"
#include "TH1F.h"
#include "TLeaf.h"
#include "TMath.h"
#include "TVector3.h"
#include "tclap/CmdLine.h"


#define NPROG 100 /// program progress step
//#define TH 3.1415/4.5 /// predefined direction around which a cone of muons is selected
#define TH 3.1415/180*15 /// predefined direction around which a cone of muons is selected
#define NDIRS 4   /// number of preselected directions
#define HALFOPENINGANGLE 15  /// helf opening angle of the sky cone in degree



using namespace std;



TVector3 DirectionVectorByAngles(double theta, double phi)
{
  double x = sin(theta)*cos(phi);
  double y = sin(theta)*sin(phi);
  double z = cos(theta);
  return TVector3(x,y,z);
}



bool InsideCone(TVector3 axis, TVector3 dir)
{
  return (axis.Angle(dir) < HALFOPENINGANGLE * TMath::Pi()/180);
}



int main(int argc, char** argv)
{
  /// variable containers
  vector<string> flist;
  int ndet = 2;  /// number of ADs
  vector<TVector3> skyDirections;
  map<int, TH1F> nRadDistsEast, nRadDistsWest, nRadDistsNorth, nRadDistsSouth;  /// neutron lateral distributions of the ith AD
  
  
  /// construct the preselected directions
  for(int i = 0; i < NDIRS; i++)
  {
    double PHI = i*TMath::TwoPi()/NDIRS;
    skyDirections.push_back(DirectionVectorByAngles(TH, PHI));
  }
  
  
  /// set up command arguments
  TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
  TCLAP::ValueArg<string> runArg("r", "run", "Specify run", false, "21221", "string");
  TCLAP::ValueArg<int> nThetaArg("t", "theta", "number of theta intervals", false, 3, "int");
  TCLAP::ValueArg<int> nPhiArg("p", "phi", "number of phi intervals", false, 6, "int");
  cmd.add(runArg);
  cmd.add(nThetaArg);
  cmd.add(nPhiArg);
  cmd.parse(argc, argv);
  
  
  /// read the needed files from the file list
  fstream inflist("filelist.txt");
  string tmpline;
  while(inflist >> tmpline)
    if(tmpline.find(runArg.getValue()) != string::npos)
      flist.push_back(tmpline);
  if(!flist.size())
  {
    cout << "no file found" << endl;
    return -1;
  }
  
  
  /// extract which hall it is
  string afile = flist[0];
  int hall = atoi(afile.substr(afile.find("EH")+2,1).c_str());
  int run = atoi(afile.substr(afile.find("Neutrino.")+9,7).c_str());
  
  
  /// open files for output
  TChain ch("/RPC-OWS/mun");
  cout << "start loading root files" << endl;
  for(unsigned int i = 0; i < flist.size(); i++)
    ch.Add(flist[i].c_str());
  system("mkdir -p output/theta15");
  
  
  /// output variables setup
  /// start making histograms
  TFile outtf(Form("output/binsky_EH%d_%d.root",hall,run), "recreate");
  if(hall == 3) ndet = 4;
  for(int i = 1; i <= ndet; i++ )
  {
    nRadDistsEast[i] = TH1F(Form("radDistEast%d",i),Form("EH%d AD%d neutron radial distribution from East muons",hall,i),200,0,1000);
    nRadDistsEast[i].GetXaxis()->SetTitle("closest approach (mm)");
    nRadDistsWest[i] = TH1F(Form("radDistWest%d",i),Form("EH%d AD%d neutron radial distribution from West muons",hall,i),200,0,1000);
    nRadDistsWest[i].GetXaxis()->SetTitle("closest approach (mm)");
    nRadDistsNorth[i] = TH1F(Form("radDistNorth%d",i),Form("EH%d AD%d neutron radial distribution from North muons",hall,i),200,0,1000);
    nRadDistsNorth[i].GetXaxis()->SetTitle("closest approach (mm)");
    nRadDistsSouth[i] = TH1F(Form("radDistSouth%d",i),Form("EH%d AD%d neutron radial distribution from South muons",hall,i),200,0,1000);
    nRadDistsSouth[i].GetXaxis()->SetTitle("closest approach (mm)");
  }
  
  
  /// event loop
  int nent = ch.GetEntries();
  cout << "finish loading root files" << endl;
  int delimiter = nent/NPROG + 1;
  for(int ent = 0; ent < nent; ent++)
  {
    //if(ent >= 400000) break;
    ch.GetEntry(ent);
    
    map<int, bool> detHitMap;
    for(int i = 1; i <= ndet; i++)
      detHitMap[i] = (bool)ch.GetLeaf(Form("inscribable%d",i))->GetValue();
    
    /// start AD loop
    for(map<int, bool>::iterator it = detHitMap.begin(); it != detHitMap.end(); it++)
    {
      /// process only if possible to inscribe a cylinder
      if(it->second)
      {
        int detId = it->first;
        ///// get the number of Gd neutrons associated with this muon
        //int nGdN = 0;
        int nInter = (int)ch.GetLeaf(Form("nIntEvtAd%d",detId))->GetValue();
        /// neutron loop
        for(int nidx = 0; nidx < nInter; nidx++)
        {
          double dt = ch.GetLeaf(Form("dtIntEvtAd%d",detId))->GetValue(nidx);
          double en = ch.GetLeaf(Form("eIntEvtAd%d",detId))->GetValue(nidx);
          double inCyl = ch.GetLeaf(Form("inCyl%d",detId))->GetValue(nidx);
          double dlNeu = ch.GetLeaf(Form("dlIntEvt%d",detId))->GetValue(nidx);
          
          /// cut for neutron selection
          if(dt > 20e-6 && dt < 1000e-6 && en > 6 && en < 12 && inCyl)
          {
            /// get the direction of the muon
            double th = ch.GetLeaf("theta")->GetValue();
            double phi = ch.GetLeaf("phi")->GetValue();
            TVector3 curDir = DirectionVectorByAngles(th, phi);
            /// fill the neutron lateral distance to the corresponding sky direction
            if(InsideCone(skyDirections[0], curDir)) nRadDistsEast[detId].Fill(dlNeu);
            if(InsideCone(skyDirections[1], curDir)) nRadDistsNorth[detId].Fill(dlNeu);
            if(InsideCone(skyDirections[2], curDir)) nRadDistsWest[detId].Fill(dlNeu);
            if(InsideCone(skyDirections[3], curDir)) nRadDistsSouth[detId].Fill(dlNeu);
          }
        }
        
      }
    }
    
    /// print program progress
    int mult = ent/delimiter;
    if(ent == mult*delimiter && mult > 0)
      cout << mult << " percent finished" << endl;
  }
  
  /// write results to the root file
  for(int i = 1; i <= ndet; i++)
  {
    nRadDistsEast[i].Write();
    nRadDistsWest[i].Write();
    nRadDistsSouth[i].Write();
    nRadDistsNorth[i].Write();
  }
  
  outtf.Close();
  
  return 0;
}
