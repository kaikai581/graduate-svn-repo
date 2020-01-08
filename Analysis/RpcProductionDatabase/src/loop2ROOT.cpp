#include <iostream>
#include <map>
#include <string>
#include "commonfunctions.h"
#include "PerCalibReadoutEvent/PerCalibReadoutHeader.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TTree.h"
#include "TROOT.h"

using namespace std;

struct rpcTime_t
{
  unsigned int sec;
  unsigned int nanoSec;
};


bool isNormalIncidence(map<unsigned int, vector<unsigned int> >);
bool is1StripPerLayerThisModule(vector<unsigned int>);


void loop2ROOT(string ifpn, string ofpn, int nEvt, bool dbgFlag)
{
  TFile tif(ifpn.c_str());
  TFile tof(ofpn.c_str(), "recreate");
  TTree *treeCalib = (TTree*)tif.Get("/Event/CalibReadout/CalibReadoutHeader");
  
  if(!treeCalib)
  {
    cout << "spallation tree not found" << endl;
    return;
  }
  
  /// dataset level storage
  PerCalibReadoutHeader* rh = 0;
  
  /// time storage
  /// the key of the map is the module ID
  /// the value is the array of trigger time of that module
  map<pair<unsigned int, unsigned int>, vector<rpcTime_t> > mTriggerTime;
  map<pair<unsigned int, unsigned int>, vector<rpcTime_t> > m3FoldTime;
  map<pair<unsigned int, unsigned int>, vector<rpcTime_t> > m4FoldTime;
  /// this variable records readout time with 4 fold and one strip each layer
  map<pair<unsigned int, unsigned int>, vector<rpcTime_t> > m4Fold1StripTime;
  
  
  /// original data model in production files
  int site;
  unsigned int triggerTimeSec;
  unsigned int triggerTimeNanoSec;
  unsigned int nHitsRpc;
  vector<unsigned int>* rpcRow;
  vector<unsigned int>* rpcColumn;
  vector<unsigned int>* rpcLayer;
  vector<unsigned int>* rpcStrip;
  vector<bool>* rpcFromRot;
  
  /// reorganized data model in this program
  /// plus some strip hit pattern variables
  /// prefix "m" means modulewise
  unsigned int nModules;
  vector<unsigned int>* nLayers = new vector<unsigned int>;
  vector<unsigned int>* mNLayers = new vector<unsigned int>;
  vector<vector<unsigned int> >* nStrips = new vector<vector<unsigned int> >;
  vector<unsigned int>* mNStrips = new vector<unsigned int>;
  vector<unsigned int>* mRpcRow = new vector<unsigned int>;
  vector<unsigned int>* mRpcColumn = new vector<unsigned int>;
  vector<vector<unsigned int> >* mRpcLayer = new vector<vector<unsigned int> >;
  vector<vector<vector<unsigned int> > >* mRpcStrip = new vector<vector<vector<unsigned int> > >;
  vector<bool>* mRpcFromRot = new vector<bool>;
  vector<bool>* mIs1Strip = new vector<bool>;
  vector<bool>* mIsNormal = new vector<bool>;
  
  /// time variables
  vector<unsigned int> *dtInNanoSec = new vector<unsigned int>;
  vector<unsigned int> *dtInNanoSec4Fold = new vector<unsigned int>;
  vector<double>* dtInSec = new vector<double>;
  vector<double>* dtInSec3Fold = new vector<double>;
  vector<double>* dtInSec4Fold = new vector<double>;
  vector<double>* dtInSec4Fold1Strip = new vector<double>;
  
  
  TTree rpcTree("rpcTree", "RPC Tree");
  rpcTree.Branch("site", &site, "site/I");
  rpcTree.Branch("triggerTimeSec", &triggerTimeSec, "triggerTimeSec/i");
  rpcTree.Branch("triggerTimeNanoSec", &triggerTimeNanoSec, "triggerTimeNanoSec/i");
  /// these leaves are direct copy from original production root files
  rpcTree.Branch("nHitsRpc", &nHitsRpc, "nHitsRpc/i");
  rpcTree.Branch("rpcRow", &rpcRow);
  rpcTree.Branch("rpcColumn", &rpcColumn);
  rpcTree.Branch("rpcLayer", &rpcLayer);
  rpcTree.Branch("rpcStrip", &rpcStrip);
  rpcTree.Branch("rpcFromRot", &rpcFromRot);
  /// these leaves are reorganized data model
  rpcTree.Branch("nModules", &nModules, "nModules/i");
  rpcTree.Branch("nLayers", &nLayers);
  rpcTree.Branch("mNLayers", &mNLayers);
  rpcTree.Branch("nStrips", &nStrips);
  rpcTree.Branch("mNStrips", &mNStrips);
  rpcTree.Branch("mRpcRow", &mRpcRow);
  rpcTree.Branch("mRpcColumn", &mRpcColumn);
  rpcTree.Branch("mRpcLayer", &mRpcLayer);
  rpcTree.Branch("mRpcStrip", &mRpcStrip);
  rpcTree.Branch("mRpcFromRot", &mRpcFromRot);
  rpcTree.Branch("mIs1Strip", &mIs1Strip);
  rpcTree.Branch("mIsNormal", &mIsNormal);
  rpcTree.Branch("dtInNanoSec", &dtInNanoSec);
  rpcTree.Branch("dtInNanoSec4Fold", &dtInNanoSec4Fold);
  rpcTree.Branch("dtInSec", &dtInSec);
  rpcTree.Branch("dtInSec3Fold", &dtInSec3Fold);
  rpcTree.Branch("dtInSec4Fold", &dtInSec4Fold);
  rpcTree.Branch("dtInSec4Fold1Strip", &dtInSec4Fold1Strip);
  
  /// start event loop
  int nentry = treeCalib->GetEntries();
  int nRpc = 0;
  for(int entry = 0; entry < nentry; entry++)
  {
    /// initialize vector variables
    nLayers->clear();
    
    /// number of events to be processed specified by -n command option
    if(nEvt>=0 && nRpc>=nEvt)
      break;
    
    treeCalib->SetBranchAddress("CalibReadout_CalibReadoutHeader", &rh);
    treeCalib->GetEntry(entry);
    if(!rh)
    {
      cout << "RPC entry not found in calibration tree" << endl;
      continue;
    }
    
    site = rh->site;
    triggerTimeSec = rh->triggerTimeSec;
    triggerTimeNanoSec = rh->triggerTimeNanoSec;
    nHitsRpc = rh->nHitsRpc;
    
    if(nHitsRpc)
      nRpc++;
    else
      continue;
    if(dbgFlag)
    {
      cout << "site: " << site << endl;
      cout << "trigger time sec: " << triggerTimeSec << "\t";
      cout << "trigger time nano sec: " << triggerTimeNanoSec << endl;
      cout << "number of hit rpc strips: " << nHitsRpc << endl;
    }
    
    rpcRow = &(rh->rpcRow);
    rpcColumn = &(rh->rpcColumn);
    rpcLayer = &(rh->rpcLayer);
    rpcStrip = &(rh->rpcStrip);
    rpcFromRot = &(rh->rpcFromRot);
    
    /// sometime nHitsRpc is nonzero while no strip information is in root file
    if(rpcRow->size() != nHitsRpc ||
       rpcColumn->size() != nHitsRpc ||
       rpcLayer->size() != nHitsRpc ||
       rpcStrip->size() != nHitsRpc ||
       rpcFromRot->size() != nHitsRpc)
    {
      cerr << "number of strips readout is inconsistent in root file" << endl;
      continue;
    }
    
    
    
    mRpcFromRot->clear();
    
    
    map<pair<unsigned int, unsigned int>, map<unsigned int, vector<unsigned int> > > readout;
    for(unsigned int sIdx = 0; sIdx < nHitsRpc; sIdx++)
    {
      pair<unsigned int, unsigned int> modId(rpcRow->at(sIdx), rpcColumn->at(sIdx));
      
      /// strip information
      unsigned int layer = rpcLayer->at(sIdx);
      unsigned int strip = rpcStrip->at(sIdx);
      
      if(readout.find(modId) == readout.end())
      {
        map<unsigned int, vector<unsigned int> > readoutPerModule;
        vector<unsigned int> readoutPerLayer(1, strip);
        readoutPerModule[layer] = readoutPerLayer;
        readout[modId] = readoutPerModule;
        mRpcFromRot->push_back(rpcFromRot->at(sIdx));
      }
      else
      {
        if(readout[modId].find(layer) == readout[modId].end())
        {
          vector<unsigned int> readoutPerLayer(1, strip);
          readout[modId][layer] = readoutPerLayer;
        }
        else
        {
          readout[modId][layer].push_back(strip);
        }
      }
      /// end of strip information
    }
    
    
    
    if(dbgFlag)
    {
      cout << "fired strip id" << endl;
      for(map<pair<unsigned int, unsigned int>, map<unsigned int, vector<unsigned int> > >::iterator mIdx = readout.begin(); mIdx != readout.end(); mIdx++)
        for(map<unsigned int, vector<unsigned int> >::iterator lIdx = (*mIdx).second.begin(); lIdx != (*mIdx).second.end(); lIdx++)
        {
          vector<unsigned int> stripThisLayer = (*lIdx).second;
          for(unsigned int j = 0; j < stripThisLayer.size(); j++)
            cout << " (" << (*mIdx).first.first << " " << (*mIdx).first.second << " " << (*lIdx).first << " " << stripThisLayer[j] << ")";
        }
      cout << endl;
    }
    
/////// fill in tree variables /////////////////////////////////////////////////
    /// clean containers
    /// strip conatiners
    nLayers->clear();
    mNLayers->clear();
    nStrips->clear();
    mNStrips->clear();
    mRpcRow->clear();
    mRpcColumn->clear();
    mRpcLayer->clear();
    mRpcStrip->clear();
    mIs1Strip->clear();
    mIsNormal->clear();
    
    /// time containers
    dtInNanoSec->clear();
    dtInNanoSec4Fold->clear();
    dtInSec->clear();
    dtInSec3Fold->clear();
    dtInSec4Fold->clear();
    dtInSec4Fold1Strip->clear();
    
    nModules = readout.size();
    rpcTime_t curTime;
    curTime.sec = triggerTimeSec;
    curTime.nanoSec = triggerTimeNanoSec;
    
    
    unsigned int mPtr = 0;
    for(map<pair<unsigned int, unsigned int>, map<unsigned int, vector<unsigned int> > >::iterator mIdx = readout.begin(); mIdx != readout.end(); mIdx++)
    {
      /// fill in strip information
      nLayers->push_back((*mIdx).second.size());
      mRpcRow->push_back((*mIdx).first.first);
      mRpcColumn->push_back((*mIdx).first.second);
      
      vector<unsigned int> nStripsPerLayer;
      vector<unsigned int> layerThisModule;
      vector<vector<unsigned int> > stripPerLayerThisModule;
      for(map<unsigned int, vector<unsigned int> >::iterator lIdx = (*mIdx).second.begin(); lIdx != (*mIdx).second.end(); lIdx++)
      {
        nStripsPerLayer.push_back((*lIdx).second.size());
        layerThisModule.push_back((*lIdx).first);
        vector<unsigned int> stripThisLayer;
        for(unsigned int sIdx = 0; sIdx < (*lIdx).second.size(); sIdx++)
          stripThisLayer.push_back((*lIdx).second[sIdx]);
        stripPerLayerThisModule.push_back(stripThisLayer);
      }
      
      mRpcLayer->push_back(layerThisModule);
      mRpcStrip->push_back(stripPerLayerThisModule);
      
      nStrips->push_back(nStripsPerLayer);
      
      
      /// fill in hit pattern characteristics
      mIs1Strip->push_back(is1StripPerLayerThisModule(nStrips->at(mPtr)));
      mIsNormal->push_back(isNormalIncidence((*mIdx).second));
      
      /// fill in time information
      pair<unsigned int, unsigned int> modId((*mIdx).first.first, (*mIdx).first.second);
      if(mRpcFromRot->at(mPtr))
      {
        /// fill in trigger time which includes 3 and 4 fold triggers
        if(mTriggerTime.find(modId) == mTriggerTime.end())
        {
          vector<rpcTime_t> dummyTime(1, curTime);
          mTriggerTime[modId] = dummyTime;
          
          dtInNanoSec->push_back(0);
          
          dtInSec->push_back(0.);
        }
        else
        {
          mTriggerTime[modId].push_back(curTime);
          unsigned int ptrCurTime = mTriggerTime[modId].size()-1;
          
          unsigned int thisDtSec = mTriggerTime[modId][ptrCurTime].sec-mTriggerTime[modId][ptrCurTime-1].sec;
          
          unsigned int thisDtInNanoSec = thisDtSec*(unsigned int)1e9+mTriggerTime[modId][ptrCurTime].nanoSec-mTriggerTime[modId][ptrCurTime-1].nanoSec;
          double thisDtInSec = (double)thisDtInNanoSec*1e-9;
          
          dtInNanoSec->push_back(thisDtInNanoSec);
          dtInSec->push_back(thisDtInSec);
        }
        
        
        /// fill in 3 fold trigger time
        if(nLayers->at(mPtr) == 3)
        {
          if(m3FoldTime.find(modId) == m3FoldTime.end())
          {
            vector<rpcTime_t> dummyTime(1, curTime);
            m3FoldTime[modId] = dummyTime;
            
            dtInSec3Fold->push_back(0.);
          }
          else
          {
            m3FoldTime[modId].push_back(curTime);
            unsigned int ptrCurTime = m3FoldTime[modId].size()-1;
            
            unsigned int thisDtSec = m3FoldTime[modId][ptrCurTime].sec-m3FoldTime[modId][ptrCurTime-1].sec;
            unsigned int thisDtInNanoSec = thisDtSec*(unsigned int)1e9+m3FoldTime[modId][ptrCurTime].nanoSec-m3FoldTime[modId][ptrCurTime-1].nanoSec;
            double thisDtInSec = (double)thisDtInNanoSec*1e-9;
            
            dtInSec3Fold->push_back(thisDtInSec);
          }
        }
        else
        {
          dtInSec3Fold->push_back(0.);
        }
        
        /// fill in 4 fold trigger time
        if(nLayers->at(mPtr) == 4)
        {
          if(m4FoldTime.find(modId) == m4FoldTime.end())
          {
            vector<rpcTime_t> dummyTime(1, curTime);
            m4FoldTime[modId] = dummyTime;
            
            dtInNanoSec4Fold->push_back(0);
            dtInSec4Fold->push_back(0.);
          }
          else
          {
            m4FoldTime[modId].push_back(curTime);
            unsigned int ptrCurTime = m4FoldTime[modId].size()-1;
            
            unsigned int thisDtSec = m4FoldTime[modId][ptrCurTime].sec-m4FoldTime[modId][ptrCurTime-1].sec;
            unsigned int thisDtInNanoSec = thisDtSec*(unsigned int)1e9+m4FoldTime[modId][ptrCurTime].nanoSec-m4FoldTime[modId][ptrCurTime-1].nanoSec;
            double thisDtInSec = (double)thisDtInNanoSec*1e-9;
            
            dtInNanoSec4Fold->push_back(thisDtInNanoSec);
            dtInSec4Fold->push_back(thisDtInSec);
          }
          
          /// fill in 4 fold and 1 strip each layer trigger time
          bool is1Strip = true;
          for(unsigned int j = 0; j < (*nStrips)[mPtr].size(); j++)
            is1Strip &= ((*nStrips)[mPtr][j] == 1);
          if(is1Strip)
          {
            if(m4Fold1StripTime.find(modId) == m4Fold1StripTime.end())
            {
              vector<rpcTime_t> dummyTime(1, curTime);
              m4Fold1StripTime[modId] = dummyTime;
              dtInSec4Fold1Strip->push_back(0.);
            }
            else
            {
              m4Fold1StripTime[modId].push_back(curTime);
              unsigned int ptrCurTime = m4Fold1StripTime[modId].size()-1;
              
              unsigned int thisDtSec = m4Fold1StripTime[modId][ptrCurTime].sec-m4Fold1StripTime[modId][ptrCurTime-1].sec;
              unsigned int thisDtInNanoSec = thisDtSec*(unsigned int)1e9+m4Fold1StripTime[modId][ptrCurTime].nanoSec-m4Fold1StripTime[modId][ptrCurTime-1].nanoSec;
              double thisDtInSec = (double)thisDtInNanoSec*1e-9;
              
              dtInSec4Fold1Strip->push_back(thisDtInSec);
            }
          }
          else
          {
            dtInSec4Fold1Strip->push_back(0.);
          }
        }
        else
        {
          dtInNanoSec4Fold->push_back(0);
          dtInSec4Fold->push_back(0.);
          dtInSec4Fold1Strip->push_back(0.);
        }
      }
      else // forced triggers
      {
        dtInNanoSec->push_back(0);
        dtInNanoSec4Fold->push_back(0);
        dtInSec->push_back(0.);
        dtInSec3Fold->push_back(0.);
        dtInSec4Fold->push_back(0.);
        dtInSec4Fold1Strip->push_back(0.);
      }
      mPtr++;
    }
    
    
/////// fill in # layers & # strips at module level ////////////////////////////
    for(unsigned int i = 0; i < nModules; i++)
    {
      unsigned int nLayersThisModule = 0;
      unsigned int nStripsThisModule = 0;
      for(unsigned int j = 0; j < nLayers->at(i); j++)
      {
        nLayersThisModule++;
        for(unsigned int k = 0; k < (*nStrips)[i][j]; k++)
          nStripsThisModule++;
      }
      mNLayers->push_back(nLayersThisModule);
      mNStrips->push_back(nStripsThisModule);
    }
    
    
    
    
    if(dbgFlag)
    {
      cout << "number of readout modules: " << nModules << endl;
      
      cout << "(row, col):";
      for(unsigned int i = 0; i < nModules; i++)
        cout << " (" << mRpcRow->at(i) << ", " << mRpcColumn->at(i) << ")";
      cout << endl;
      
      cout << "number of readout layers:";
      for(unsigned int i = 0; i < nModules; i++)
        cout << " " << nLayers->at(i);
      cout << endl;
      
      cout << "layer id:";
      for(unsigned int i = 0; i < nModules; i++)
      {
        cout << " (";
        for(unsigned int j = 0; j < nLayers->at(i); j++)
        {
          cout << (*mRpcLayer)[i][j];
          if(j != nLayers->at(i)-1)
            cout << " ";
          else
            cout << ")";
        }
      }
      cout << endl;
      
      cout << "number of strips per layer:";
      for(unsigned int i = 0; i < nModules; i++)
      {
        cout << " (";
        for(unsigned int j = 0; j < nLayers->at(i); j++)
        {
          cout << (*nStrips)[i][j];
          if(j != nLayers->at(i)-1)
            cout << " ";
          else
            cout << ")";
        }
      }
      cout << endl;
      
      cout << "RPC row   :";
      for(unsigned int i = 0; i < nHitsRpc; i++)
        cout << " " << rpcRow->at(i);
      cout << endl;
      
      cout << "RPC column:";
      for(unsigned int i = 0; i < nHitsRpc; i++)
        cout << " " << rpcColumn->at(i);
      cout << endl;
      
      cout << "RPC layer :";
      for(unsigned int i = 0; i < nHitsRpc; i++)
        cout << " " << rpcLayer->at(i);
      cout << endl;
      
      cout << "RPC strip :";
      for(unsigned int i = 0; i < nHitsRpc; i++)
        cout << " " << rpcStrip->at(i);
      cout << endl;
      
      cout << "interarrival time of 3 fold triggers:" << endl;
      for(unsigned int i = 0; i < dtInSec3Fold->size(); i++)
        cout << dtInSec3Fold->at(i) << " ";
      cout << endl;
      
      cout << "interarrival time of 4 fold & 1 strip per layer:" << endl;
      for(unsigned int i = 0; i < dtInSec4Fold1Strip->size(); i++)
        cout << dtInSec4Fold1Strip->at(i) << " ";
      cout << endl;
      
      cout << "one strip per layer? ";
      for(unsigned int i = 0; i < mIs1Strip->size(); i++)
        cout << mIs1Strip->at(i) << " ";
      cout << endl;
      
      cout << "normal incidence? ";
      for(unsigned int i = 0; i < mIsNormal->size(); i++)
        cout << mIsNormal->at(i) << " ";
      cout << endl;
      
      cout << endl;
    }
    
    tof.cd();
    rpcTree.Fill();
  }
  
  
  if(rpcTree.GetEntries())
  {
    tof.cd();
    rpcTree.Write();
    tof.Close();
  }
  else
  {
    noRpcList(ifpn);
    tof.Close();
    gROOT->ProcessLine(Form(".! rm -f %s", ofpn.c_str()));
  }
}



bool isNormalIncidence(map<unsigned int, vector<unsigned int> > hitPattern)
{
  
  unsigned int layerHitPattern[4] = {100};
  
  /// if any layer contains more than one strip, it's regarded as not normal
  for(map<unsigned int, vector<unsigned int> >::iterator it = hitPattern.begin(); it != hitPattern.end(); it++)
    if((*it).second.size() > 1) return false;
    else
      layerHitPattern[(*it).first-1] = (*it).second[0];
  
  
  unsigned int nLayers = hitPattern.size();
  
  if(nLayers <= 3) // 3 fold + forced trigger
  {
    if(layerHitPattern[0] == layerHitPattern[3]) return true;
    if(layerHitPattern[1] == layerHitPattern[2]) return true;
  }
  else // 4 fold
  {
    if(layerHitPattern[0] != layerHitPattern[3]) return false;
    if(layerHitPattern[1] != layerHitPattern[2]) return false;
  }
  
  return true;
}



bool is1StripPerLayerThisModule(vector<unsigned int> nStripsThisModule)
{
  bool result = true;
  for(unsigned int i = 0; i < nStripsThisModule.size(); i++)
    result &= (nStripsThisModule[i] == 1);
  return result;
}
