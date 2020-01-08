/*
 * To make a file list, one can use the following command:
 * $ find /global/project/projectdirs/dayabay/scratch/sklin/RootFile/MuonLookupTable/MuonInducedNeutron -type f|xargs ls > filelist.txt
 * A much faster command although unsorted is:
 * $ find /global/project/projectdirs/dayabay/scratch/sklin/RootFile/MuonLookupTable/MuonInducedNeutron -type f > filelist.txt
 * 
 * Oct 2013 by Shih-Kai.
 */


#include <fstream>
#include <iostream>
#include "Math/Point3D.h"
#include "Math/Vector3D.h"
#include "TChain.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1F.h"
#include "TLeaf.h"
#include "TMath.h"
#include "TParameter.h"
#include "TVector3.h"
#include "tclap/CmdLine.h"

#define NPROG 100 /// program progress step


using namespace ROOT::Math;
using namespace std;



/// fit the lateral distribution of the neutrons to a 2D Gaussian
double fitLateralDistribution(TH1F& hist)
{
  TF1 f("f", "[0]*x*exp(-x*x/2/[1]/[1])", 0, 1000);
  f.SetParameters(100,500);
  hist.Fit(&f);
  
  double width = f.GetParameter(1);
  
  return 1-exp(-1000*1000/2/width/width);
}


/// get the parameter of a point along the track
double getParAlongTrack(TVector3 p, TVector3 p0, TVector3 dir)
{
  TVector3 p0p = p - p0;
  double t = p0p.Mag()*cos(p0p.Angle(dir));
  return t;
}


int main(int argc, char** argv)
{
  /// local variables
  vector<int> detlist;
  map<int, double> detTrackLength;
  map<int, int> detNMuon;
  map<int, int> detNNeu;
  map<int, TH1F> detNRadDist, detNRadDistOffWindow, detNRadOutCyl;
  map<int, TH1F> detNRadDistHighLoss, detNRadDistLowLoss; /// cut on dE/dx
  map<int, TH1F> detNeuTime;
  map<int, TH1F> detNeuE, detNeuEOffWindow;
  map<int, TH1F> detdEdx;
  /// variables for sectioned sky; partition the sky into east and west
  map<int, double> detTrackLengthEast, detTrackLengthWest;
  map<int, int> detNNeuEast, detNNeuWest;
  map<int, TH1F> detNRadDistEast, detNRadDistWest;
  map<int, TH1F> detNRadDistEastThCut, detNRadDistWestThCut;
  
  /// hall rotation angles
  map<int, double> rotationAngles; // in radian
  rotationAngles[1] = -2.145;
  rotationAngles[2] = 1.3912;
  rotationAngles[3] = 2.627593;
  /// detector origins
  map<int, TVector3> eh1origins, eh2origins, eh3origins;
  eh1origins[1] = TVector3(-18079.5, -799699, -7092.5);
  eh1origins[2] = TVector3(-14960.5, -804521, -7092.5);
  eh2origins[1] = TVector3(472323, 60325.2, -3712.5);
  eh2origins[2] = TVector3(471297, 65974.8, -3712.5);
  eh3origins[1] = TVector3(-406758, 812082, -2242.5);
  eh3origins[2] = TVector3(-411758, 809258, -2242.5);
  eh3origins[3] = TVector3(-409582, 817082, -2242.5);
  eh3origins[4] = TVector3(-414582, 814258, -2242.5);
  map<int, map<int, TVector3> > origins;
  origins[1] = eh1origins;
  origins[2] = eh2origins;
  origins[3] = eh3origins;
  
  /// set up command arguments
  TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
  TCLAP::ValueArg<string> hallArg("a", "hall", "Specify hall", false, "1", "string");
  TCLAP::ValueArg<string> pathArg("p", "path", "Specify path", false, "/global/project/projectdirs/dayabay/scratch/sklin/RootFile/MuonLookupTable/MuonInducedNeutron/2011/p12e/Neutrino/1224", "string");
  TCLAP::ValueArg<string> runArg("r", "run", "Specify run", false, "", "string");
  cmd.add(hallArg);
  cmd.add(pathArg);
  cmd.add(runArg);
  cmd.parse(argc, argv);
  
  /// read the needed files from the file list
  fstream inflist("filelist.txt");
  string tmpline;
  vector<string> flist;
  int selrule = 1; /// select with file path and hall number
  if(runArg.getValue() != "") selrule = 0; /// select with run # only
  while(inflist >> tmpline)
  {
    if(selrule == 0)
      if(tmpline.find(runArg.getValue()) != string::npos)
        flist.push_back(tmpline);
    
    if(selrule == 1)
    {
      string hallStr = "EH" + hallArg.getValue();
      string selStr = pathArg.getValue();
      if(tmpline.find(pathArg.getValue()) != string::npos)
        if(tmpline.find(hallStr) != string::npos)
          flist.push_back(tmpline);
    }
  }
  
  if(!flist.size())
  {
    cout << "no file found" << endl;
    return -1;
  }
  
  /// extract which hall it is
  string afile = flist[0];
  int hall = atoi(afile.substr(afile.find("EH")+2,1).c_str());
  int run = atoi(afile.substr(afile.find("Neutrino.")+9,7).c_str());

  
  TChain ch("/RPC-OWS/mun");
  cout << "start loading root files" << endl;
  for(unsigned int i = 0; i < flist.size(); i++)
    ch.Add(flist[i].c_str());
  cout << "finish loading root files" << endl;
  
  /// start making histograms
  TFile outtf(Form("output/EH%d_%d.root",hall,run), "recreate");
  /// Tree for neutron position
  TTree trNeu("trNeu", "neutron data regardless of cylinder");
  /// neutron tree variables
  int adNeu, inCylNeu, trigNum, canFitCyl;
  double tCylUp, tCylBot, tCylMid;
  double xNeu, yNeu, zNeu, latNeu;
  double xlNeu, ylNeu, zlNeu; // local coordinate
  double zAxScaled;
  double dtNeu, tNeu;
  trNeu.Branch("adNeu", &adNeu, "adNeu/I");
  trNeu.Branch("trigNum", &trigNum, "trigNum/I");
  trNeu.Branch("canFitCyl", &canFitCyl, "canFitCyl/I");
  trNeu.Branch("tCylUp", &tCylUp, "tCylUp/D");
  trNeu.Branch("tCylBot", &tCylBot, "tCylBot/D");
  trNeu.Branch("tCylMid", &tCylMid, "tCylMid/D");
  
  trNeu.Branch("inCylNeu", &inCylNeu, "inCylNeu/I");
  trNeu.Branch("xNeu", &xNeu, "xNeu/D");
  trNeu.Branch("yNeu", &yNeu, "yNeu/D");
  trNeu.Branch("zNeu", &zNeu, "zNeu/D");
  trNeu.Branch("xlNeu", &xlNeu, "xlNeu/D");
  trNeu.Branch("ylNeu", &ylNeu, "ylNeu/D");
  trNeu.Branch("zlNeu", &zlNeu, "zlNeu/D");
  trNeu.Branch("latNeu", &latNeu, "latNeu/D");
  trNeu.Branch("zAxScaled", &zAxScaled, "zAxScaled/D");
  trNeu.Branch("dtNeu", &dtNeu, "dtNeu/D");
  trNeu.Branch("tNeu", &tNeu, "tNeu/D");
  
  /// all inter muon events without neutron cut
  TTree trInt("trInt", "inter muon event data");
  double enNeu;
  trInt.Branch("adNeu", &adNeu, "adNeu/I");
  trInt.Branch("enNeu", &enNeu, "enNeu/D");
  trInt.Branch("trigNum", &trigNum, "trigNum/I");
  trInt.Branch("canFitCyl", &canFitCyl, "canFitCyl/I");
  trInt.Branch("tCylUp", &tCylUp, "tCylUp/D");
  trInt.Branch("tCylBot", &tCylBot, "tCylBot/D");
  trInt.Branch("tCylMid", &tCylMid, "tCylMid/D");
  
  trInt.Branch("inCylNeu", &inCylNeu, "inCylNeu/I");
  trInt.Branch("xNeu", &xNeu, "xNeu/D");
  trInt.Branch("yNeu", &yNeu, "yNeu/D");
  trInt.Branch("zNeu", &zNeu, "zNeu/D");
  trInt.Branch("xlNeu", &xlNeu, "xlNeu/D");
  trInt.Branch("ylNeu", &ylNeu, "ylNeu/D");
  trInt.Branch("zlNeu", &zlNeu, "zlNeu/D");
  trInt.Branch("latNeu", &latNeu, "latNeu/D");
  trInt.Branch("zAxScaled", &zAxScaled, "zAxScaled/D");
  trInt.Branch("dtNeu", &dtNeu, "dtNeu/D");
  trInt.Branch("tNeu", &tNeu, "tNeu/D");
  
  /// initialize variables
  int ndet = 2;
  if(hall == 3) ndet = 4;
  for(int i = 1; i <= ndet; i++ )
  {
    detNMuon[i] = 0;
    detTrackLength[i] = 0;
    detNNeu[i] = 0;
    detNRadDist[i] = TH1F(Form("radDist%d",i),Form("EH%d AD%d neutron radial distribution",hall,i),200,0,1000);
    detNRadDist[i].GetXaxis()->SetTitle("closest approach (mm)");
    detNRadOutCyl[i] = TH1F(Form("radDistOutCyl%d",i),Form("EH%d AD%d neutron radial distribution no cylinder",hall,i),200,0,15000);
    detNRadOutCyl[i].GetXaxis()->SetTitle("closest approach (mm)");
    detNRadDistOffWindow[i] = TH1F(Form("radDistOff%d",i),Form("EH%d AD%d off window neutron radial distribution",hall,i),200,0,1000);
    detNRadDistOffWindow[i].GetXaxis()->SetTitle("closest approach (mm)");
    detNRadDistHighLoss[i] = TH1F(Form("radDist%dHighLoss",i),Form("EH%d AD%d neutron radial distribution with high muon energy loss",hall,i),200,0,1000);
    detNRadDistHighLoss[i].GetXaxis()->SetTitle("closest approach (mm)");
    detNRadDistLowLoss[i] = TH1F(Form("radDist%dLowLoss",i),Form("EH%d AD%d neutron radial distribution with low muon energy loss",hall,i),200,0,1000);
    detNRadDistLowLoss[i].GetXaxis()->SetTitle("closest approach (mm)");
    detNeuTime[i] = TH1F(Form("dt%d",i),Form("EH%d AD%d neutron capture time",hall,i),500,0,1000);
    detNeuTime[i].GetXaxis()->SetTitle("capture time (#mus)");
    detNeuE[i] = TH1F(Form("E%d",i),Form("EH%d AD%d neutron energy spectrum",hall,i),500,6,12);
    detNeuE[i].GetXaxis()->SetTitle("energy (MeV)");
    detNeuEOffWindow[i] = TH1F(Form("EOff%d",i),Form("EH%d AD%d off window neutron energy spectrum",hall,i),500,6,12);
    detNeuEOffWindow[i].GetXaxis()->SetTitle("energy (MeV)");
    detdEdx[i] = TH1F(Form("dEdx%d",i),Form("EH%d AD%d muon dE/dx",hall,i),500,0,20);
    detdEdx[i].GetXaxis()->SetTitle("energy loss (MeV/cm)");
    /// initialize the partitioned variables
    detTrackLengthEast[i] = 0;
    detTrackLengthWest[i] = 0;
    detNNeuEast[i] = 0;
    detNNeuWest[i] = 0;
    detNRadDistEast[i] = TH1F(Form("radDist%dEast",i),Form("EH%d AD%d neutron radial distribution from east muons",hall,i),200,0,1000);
    detNRadDistWest[i] = TH1F(Form("radDist%dWest",i),Form("EH%d AD%d neutron radial distribution from west muons",hall,i),200,0,1000);
    detNRadDistEastThCut[i] = TH1F(Form("radDist%dEastThCut",i),Form("EH%d AD%d neutron radial distribution from east muons",hall,i),200,0,1000);
    detNRadDistWestThCut[i] = TH1F(Form("radDist%dWestThCut",i),Form("EH%d AD%d neutron radial distribution from west muons",hall,i),200,0,1000);
    detNRadDistEast[i].GetXaxis()->SetTitle("closest approach (mm)");
    detNRadDistWest[i].GetXaxis()->SetTitle("closest approach (mm)");
    detNRadDistEastThCut[i].GetXaxis()->SetTitle("closest approach (mm)");
    detNRadDistWestThCut[i].GetXaxis()->SetTitle("closest approach (mm)");
  }
  
  int nent = ch.GetEntries();
  int delimiter = nent/NPROG + 1;
  for(int ent = 0; ent < nent; ent++)
  {
    //if(ent >= 400000) break;
    ch.GetEntry(ent);
    
    map<int, bool> detHitMap;
    for(int i = 1; i <= ndet; i++)
      detHitMap[i] = (bool)ch.GetLeaf(Form("inscribable%d",i))->GetValue();
    
    for(map<int, bool>::iterator it = detHitMap.begin(); it != detHitMap.end(); it++)
    {
      /// process only if possible to inscribe a cylinder
      if(it->second)
      {
      
        int detId = it->first;
        double curTrLen = ch.GetLeaf(Form("tCyl%dUp",detId))->GetValue()-ch.GetLeaf(Form("tCyl%dBot",detId))->GetValue();
        detTrackLength[detId] += curTrLen;
        detNMuon[detId]++;
        /// get track point
        double xCt = ch.GetLeaf("xCt")->GetValue();
        double yCt = ch.GetLeaf("yCt")->GetValue();
        double zCt = ch.GetLeaf("zCt")->GetValue();
        double th = ch.GetLeaf("theta")->GetValue();
        double phi = ch.GetLeaf("phi")->GetValue();
        XYZPoint pCt(xCt,yCt,zCt);
        XYZVector vDir(sin(th)*cos(phi),sin(th)*sin(phi),cos(th));
        
        /// partition the sky
        double phidegree = phi*180/TMath::Pi();
        if(phidegree > -90 && phidegree < 90)
          detTrackLengthEast[detId] += curTrLen;
        else
          detTrackLengthWest[detId] += curTrLen;
        
        /// loop for neutrons
        int nneu = (int)ch.GetLeaf(Form("nIntEvtAd%d",detId))->GetValue();
        int nGdN = 0;
        double emu = ch.GetLeaf(Form("eAd%d",detId))->GetValue();
        double dlOav = ch.GetLeaf(Form("dlOav%d",detId))->GetValue();
        double energyLoss = emu/dlOav*10; /// in unit MeV/cm
        for(int nidx = 0; nidx < nneu; nidx++)
        {
          double dt = ch.GetLeaf(Form("dtIntEvtAd%d",detId))->GetValue(nidx);
          double en = ch.GetLeaf(Form("eIntEvtAd%d",detId))->GetValue(nidx);
          double inCyl = ch.GetLeaf(Form("inCyl%d",detId))->GetValue(nidx);
          double dlNeu = ch.GetLeaf(Form("dlIntEvt%d",detId))->GetValue(nidx);
          
          
          /// cut for neutron selection
          if(dt > 20e-6 && dt < 1000e-6 && en > 6 && en < 12 && inCyl)
          {
            /// increment number of neutron
            nGdN++;
            
            detNNeu[detId]++;
            detNRadDist[detId].Fill(dlNeu);
            detNeuTime[detId].Fill(dt*1e6);
            detNeuE[detId].Fill(en);
            
            /// partition the sky
            if(phidegree > -90 && phidegree < 90)
            {
              detNNeuEast[detId]++;
              detNRadDistEast[detId].Fill(dlNeu);
              if(cos(th)>0.707) detNRadDistEastThCut[detId].Fill(dlNeu);
            }
            else
            {
              detNNeuWest[detId]++;
              detNRadDistWest[detId].Fill(dlNeu);
              if(cos(th)>0.707) detNRadDistWestThCut[detId].Fill(dlNeu);
            }
            
            /// 2 lateral distribution according to dE/dx
            /// tentative cut at dE/dx = 4
            if(energyLoss >= 4) detNRadDistHighLoss[detId].Fill(dlNeu);
            else                detNRadDistLowLoss[detId].Fill(dlNeu);
          }
          
          /// off window neutrons
          if(dt > 1020e-6 && dt < 2000e-6 && en > 6 && en < 12 && inCyl)
          {
            detNeuEOffWindow[detId].Fill(en);
            detNRadDistOffWindow[detId].Fill(dlNeu);
          }
        }
        
        /// if there are neutrons associated with this muon record the muon's
        /// energy loss.
        if(nGdN)
          detdEdx[detId].Fill(energyLoss);
        
      }
    }
    
    
    /* neutron lateral distribution regardless fiducial volume */
    int nTracks = (int)ch.GetLeaf("nTracks")->GetValue();
    if(nTracks == 1)
    {
      /// get track point
      double xCt = ch.GetLeaf("xCt")->GetValue();
      double yCt = ch.GetLeaf("yCt")->GetValue();
      double zCt = ch.GetLeaf("zCt")->GetValue();
      double th = ch.GetLeaf("theta")->GetValue();
      double phi = ch.GetLeaf("phi")->GetValue();
      TVector3 pCt(xCt,yCt,zCt);
      TVector3 vDir(sin(th)*cos(phi),sin(th)*sin(phi),cos(th));
      for(int detId = 1; detId <= ndet; detId++)
      {
        canFitCyl = (int)ch.GetLeaf(Form("inscribable%d",detId))->GetValue();
        tCylUp = 0, tCylBot = 0;
        if(canFitCyl)
        {
          tCylUp = ch.GetLeaf(Form("tCyl%dUp",detId))->GetValue();
          tCylBot = ch.GetLeaf(Form("tCyl%dBot",detId))->GetValue();
        }
        tCylMid = (tCylUp + tCylBot)/2;
        
        /// loop for neutrons
        int nneu = (int)ch.GetLeaf(Form("nIntEvtAd%d",detId))->GetValue();
        for(int nidx = 0; nidx < nneu; nidx++)
        {
          double dt = ch.GetLeaf(Form("dtIntEvtAd%d",detId))->GetValue(nidx);
          double en = ch.GetLeaf(Form("eIntEvtAd%d",detId))->GetValue(nidx);
          double dlNeu = ch.GetLeaf(Form("dlIntEvt%d",detId))->GetValue(nidx);
          
          /// record all inter muon events
          enNeu = en;
          adNeu = detId;
          trigNum = (int)ch.GetLeaf(Form("tnIntEvtAd%d",detId))->GetValue(nidx);
          inCylNeu = (int)ch.GetLeaf(Form("inCyl%d",detId))->GetValue(nidx);
          latNeu = dlNeu;
          xNeu = ch.GetLeaf(Form("xIntEvtAd%d",detId))->GetValue(nidx);
          yNeu = ch.GetLeaf(Form("yIntEvtAd%d",detId))->GetValue(nidx);
          zNeu = ch.GetLeaf(Form("zIntEvtAd%d",detId))->GetValue(nidx);
          dtNeu = ch.GetLeaf(Form("dtIntEvtAd%d",detId))->GetValue(nidx);
          TVector3 gp(xNeu, yNeu, zNeu);
          TVector3 lp;
          lp = gp - TVector3(origins[hall][detId]);
          lp.RotateZ(rotationAngles[hall]);
          xlNeu = lp.X();
          ylNeu = lp.Y();
          zlNeu = lp.Z();
          tNeu = getParAlongTrack(gp, pCt, vDir);
          zAxScaled = 0;
          if(tCylUp - tCylBot > 0)
            zAxScaled = (tNeu - tCylMid)/(tCylUp - tCylBot);
          trInt.Fill();
          
          /// cut for neutron selection
          if(dt > 20e-6 && dt < 1000e-6 && en > 6 && en < 12)
          {
            detNRadOutCyl[detId].Fill(dlNeu);
            
            /// write position to neutron tree
            //adNeu = detId;
            //trigNum = (int)ch.GetLeaf(Form("tnIntEvtAd%d",detId))->GetValue(nidx);
            //inCylNeu = (int)ch.GetLeaf(Form("inCyl%d",detId))->GetValue(nidx);
            //latNeu = dlNeu;
            //xNeu = ch.GetLeaf(Form("xIntEvtAd%d",detId))->GetValue(nidx);
            //yNeu = ch.GetLeaf(Form("yIntEvtAd%d",detId))->GetValue(nidx);
            //zNeu = ch.GetLeaf(Form("zIntEvtAd%d",detId))->GetValue(nidx);
            //dtNeu = ch.GetLeaf(Form("dtIntEvtAd%d",detId))->GetValue(nidx);
            //TVector3 gp(xNeu, yNeu, zNeu);
            //TVector3 lp;
            //lp = gp - TVector3(origins[hall][detId]);
            //lp.RotateZ(rotationAngles[hall]);
            //xlNeu = lp.X();
            //ylNeu = lp.Y();
            //zlNeu = lp.Z();
            //tNeu = getParAlongTrack(gp, pCt, vDir);
            //zAxScaled = 0;
            //if(tCylUp - tCylBot > 0)
              //zAxScaled = (tNeu - tCylMid)/(tCylUp - tCylBot);
            trNeu.Fill();
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
    cout << detTrackLength[i] << " " << detNNeu[i] << " " << detNNeu[i]/detTrackLength[i] << endl;
    TParameter<double> pTotTrLen(Form("totTrLen%d",i), detTrackLength[i]);
    TParameter<double> pTotTrLenEast(Form("totTrLen%dEast",i), detTrackLengthEast[i]);
    TParameter<double> pTotTrLenWest(Form("totTrLen%dWest",i), detTrackLengthWest[i]);
    TParameter<int> pNNeu(Form("nNeu%d",i), detNNeu[i]);
    TParameter<int> pNNeuEast(Form("nNeu%dEast",i), detNNeuEast[i]);
    TParameter<int> pNNeuWest(Form("nNeu%dWest",i), detNNeuWest[i]);
    /// lateral acceptance
    TParameter<double> pLatAcc(Form("latAcc%d",i), fitLateralDistribution(detNRadDist[i]));
    TParameter<double> pLatAccEast(Form("latAccEast%d",i), fitLateralDistribution(detNRadDistEast[i]));
    TParameter<double> pLatAccWest(Form("latAccWest%d",i), fitLateralDistribution(detNRadDistWest[i]));
    
    
    /// write results to the output file
    pTotTrLen.Write();
    pTotTrLenEast.Write();
    pTotTrLenWest.Write();
    pNNeu.Write();
    pNNeuEast.Write();
    pNNeuWest.Write();
    pLatAcc.Write();
    pLatAccEast.Write();
    pLatAccWest.Write();
    detNRadDist[i].Write();
    detNRadOutCyl[i].Write();
    detNRadDistHighLoss[i].Write();
    detNRadDistLowLoss[i].Write();
    detNRadDistEast[i].Write();
    detNRadDistWest[i].Write();
    detNRadDistEastThCut[i].Write();
    detNRadDistWestThCut[i].Write();
    detNeuTime[i].Write();
    detNeuE[i].Write();
    detNeuEOffWindow[i].Write();
    detdEdx[i].Write();
  }
  trNeu.Write();
  trInt.Write();

  outtf.Close();
  
  return 0;
}
