#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "TFile.h"
#include "TH2I.h"
#include "TH2F.h"
#include "TROOT.h"

using namespace std;


template<class T> T** allocModulewise(int, int);
template<class T> T*** allocLayerwise(int, int);
template<class T> void deleteModulewise(T**, int);
template<class T> void deleteLayerwise(T***, int, int);
void initArray(int***, int, int);
void initArray(int**, int, int);
void initArray(float***, int, int);
void initArray(float**, int, int);
void getLayerEffErr(int**, int***, float***, int, int);
void getModuleEffErr(float**, float***, float***, int, int);
bool fexists(const char*);


/// global veriables for data path
string year[] = {"2012"};
vector<string> vecYr(year, year+sizeof(year)/sizeof(string));
string productionVer = "p12b";
string myDataPath = "/eliza7/dayabay/scratch/sklin/RootFile/RpcEffRec";


/// first argument: hall number
void EffVsTime(int hn = 1)
{
  /// determine output file name according to input parameters
  stringstream outName;
  outName << "eff.EH" << hn << ".root";
  TFile effOut(outName.str().c_str(), "recreate");
  
  for(unsigned int yrIdx = 0; yrIdx < vecYr.size(); yrIdx++)
  {
    stringstream dataYearPath;
    dataYearPath << myDataPath << "/" << vecYr[yrIdx] << "/";
    dataYearPath << productionVer << "/Neutrino";
    gROOT->ProcessLine(Form(".! ls %s > infdir", dataYearPath.str().c_str()));
    ifstream ifd("infdir", ifstream::in);
    string infDate;
    
    
    /// loop over all days
    while(ifd >> infDate)
    {
      stringstream dataDatePath;
      dataDatePath << dataYearPath.str() << "/" << infDate;
      string infn;
      gROOT->ProcessLine(Form(".! ls %s/*EH%d*.root > inflist",
                        dataDatePath.str().c_str(), hn));
      ifstream ifs("inflist", ifstream::in);
      int ***totLayerHitCount = 0, **totPassMuonCount = 0;
      float ***layerEff = 0, **modEff = 0;
      float ***layerEffErr = 0, **modEffErr = 0;
      int maxHitCount = 0;
      int minHitCount = INT_MAX;
      int ncols = 0, nrows = 0;
      TH2I *layerCount[4];
      TH2I *muonCount = 0;
      
      /// start accumulating data in one day
      while(ifs >> infn)
      {
        cout << "opening file: " << endl;
        cout << infn << endl;
        TFile inf(infn.c_str(), "read");
        for(int i = 0; i < 4; i++)
          layerCount[i] = (TH2I*)inf.Get(Form("l%d",i));
        muonCount = (TH2I*)inf.Get("m");
        
        ncols = muonCount->GetNbinsX();
        nrows = muonCount->GetNbinsY();
        /// layer counts in one dataset
        int ***layerHitCount = 0, **passMuonCount = 0;
        
        /// results memory allocation and initialization
        if(!totLayerHitCount&&!totPassMuonCount&&!layerEff&&!modEff)
        {
          totLayerHitCount = new int** [nrows];
          layerEff = new float** [nrows];
          totPassMuonCount = new int* [nrows];
          modEff = new float* [nrows];
          
          for(int i = 0; i < nrows; i++)
          {
            totLayerHitCount[i] = new int* [ncols];
            layerEff[i] = new float* [ncols];
            totPassMuonCount[i] = new int [ncols];
            modEff[i] = new float [ncols];
            for(int j = 0; j < ncols; j++)
            {
              totPassMuonCount[i][j] = 0;
              modEff[i][j] = 0.;
              totLayerHitCount[i][j] = new int [4];
              layerEff[i][j] = new float [4];
              for(int k = 0; k < 4; k++)
              {
                totLayerHitCount[i][j][k] = 0;
                layerEff[i][j][k] = 0.;
              }
            }
          }
        }
        
        /// memory allocation for errors
        if(!layerEffErr)
        {
          layerEffErr = allocLayerwise<float>(nrows, ncols);
          initArray(layerEffErr, nrows, ncols);
        }
        if(!modEffErr)
        {
          modEffErr = allocModulewise<float>(nrows, ncols);
          initArray(modEffErr, nrows, ncols);
        }
        
        if(!layerHitCount&&!passMuonCount)
        {
          /// row runs from 0..7 for near sites; 0..10 for far site
          /// column runs from 1..9
          
          layerHitCount = new int** [nrows];
          passMuonCount = new int* [nrows];
          
          for(int i = 0; i < nrows; i++)
          {
            layerHitCount[i] = new int* [ncols];
            passMuonCount[i] = new int [ncols];
            for(int j = 0; j < ncols; j++)
              layerHitCount[i][j] = new int [4];
          }
        }
        for(int i = 0; i < nrows; i++)
          for(int j = 0; j < ncols; j++)
          {
            passMuonCount[i][j] = (int)muonCount->GetBinContent(j+1, i+1);
    
            totPassMuonCount[i][j] += passMuonCount[i][j];
            if(totPassMuonCount[i][j] > maxHitCount)
              maxHitCount = totPassMuonCount[i][j];
          }
        
    
    
        /// add up layer hit count
        for(int k = 0; k < 4; k++)
        {
          for(int i = 0; i < nrows; i++)
          {
            for(int j = 0; j < ncols; j++)
            {
              layerHitCount[i][j][k] = (int)layerCount[k]->
                                        GetBinContent(j+1, i+1);
              totLayerHitCount[i][j][k] += layerHitCount[i][j][k];
              if(totLayerHitCount[i][j][k] < minHitCount &&
                totLayerHitCount[i][j][k] > 0)
                minHitCount = totLayerHitCount[i][j][k];
            }
          }
        }
        
        deleteModulewise(passMuonCount, nrows);
        deleteLayerwise(layerHitCount, nrows, ncols);
        
      }
      
      /// calculate layer efficiency
      float layerMinEff = 1.;
      cout << "layer efficiency: " << endl;
      for(int k = 0; k < 4; k++)
      {
        cout << "layer " << k << ":" << endl;
        for(int i = 0; i < nrows; i++)
        {
          for(int j = 0; j < ncols; j++)
          {
            if(totPassMuonCount[i][j])
              layerEff[i][j][k] = (float)totLayerHitCount[i][j][k]
                                  /totPassMuonCount[i][j];
            else
              layerEff[i][j][k] = 0.;
            
            cout << layerEff[i][j][k] << " ";
            if(layerEff[i][j][k] < layerMinEff && layerEff[i][j][k] > 0.)
              layerMinEff = layerEff[i][j][k];
          }
//          cout << endl;
        }
//        cout << endl;
      }
      
      /// calculate module efficiency
      float minEff = 1.;
      float avgEff = 0.;
      int nMod = 0;
      for(int i = 0; i < nrows; i++)
      {
        for(int j = 0; j < ncols; j++)
        {
          float fourProd = 1.;
          for(int k = 0; k < 4; k++)
          {
            float threeProd = (1-layerEff[i][j][k%4]);
            for(int l = 1; l < 4; l++)
              threeProd *= layerEff[i][j][(k+l)%4];
            modEff[i][j] += threeProd;

            fourProd *= layerEff[i][j][k];
          }
          modEff[i][j] += fourProd;
//          cout << modEff[i][j] << " ";
          if(modEff[i][j] < minEff && modEff[i][j] > 0.)
            minEff = modEff[i][j];
          if(modEff[i][j] > 0.)
          {
            nMod++;
            avgEff += modEff[i][j];
          }
        }
        cout << endl;
      }
      if(nMod > 0)
        avgEff /= (float)nMod;
      else
      {
        cout << "no module with efficiency > 0" << endl;
        return;
      }
      
      
      getLayerEffErr(totPassMuonCount, totLayerHitCount,
                    layerEffErr, nrows, ncols);
      getModuleEffErr(modEffErr, layerEff, layerEffErr, nrows, ncols);
      
      TH2F dailyEff(Form("eff%s%s", vecYr[yrIdx].c_str(), infDate.c_str()),
           Form("efficiency on %s%s", vecYr[yrIdx].c_str(), infDate.c_str()),
           ncols, 0, ncols, nrows, 0, nrows);
      for(int i = 0; i < nrows; i++)
        for(int j = 0; j < ncols; j++)
        {
          dailyEff.SetBinContent(j+1, i+1, modEff[i][j]);
          dailyEff.SetBinError(j+1, i+1, modEffErr[i][j]);
        }
      effOut.cd();
      dailyEff.Write();
      
      deleteModulewise(totPassMuonCount, nrows);
      deleteModulewise(modEff, nrows);
      deleteModulewise(modEffErr, nrows);
      deleteLayerwise(totLayerHitCount, nrows, ncols);
      deleteLayerwise(layerEff, nrows, ncols);
      deleteLayerwise(layerEffErr, nrows, ncols);
      
    }
    
  }
  effOut.Close();
  
  if(fexists("infdir"))
    gROOT->ProcessLine(".! rm -f infdir");
  if(fexists("inflist"))
    gROOT->ProcessLine(".! rm -f inflist");
  
  return;
}



bool fexists(const char *filename)
{
  ifstream ifile(filename);
  return ifile;
}


template<class T> T** allocModulewise(int nrows, int ncols)
{
  T** retPtr;
  retPtr = new T* [nrows];
  for(int i = 0; i < nrows; i++)
    retPtr[i] = new T [ncols];
  return retPtr;
}


template<class T> T*** allocLayerwise(int nrows, int ncols)
{
  T*** retPtr;
  retPtr = new T** [nrows];
  for(int i = 0; i < nrows; i++)
  {
    retPtr[i] = new T* [ncols];
    for(int j = 0; j < ncols; j++)
      retPtr[i][j] = new T [4];
  }
  return retPtr;
}


template<class T> void deleteModulewise(T** arrayPtr, int nrows)
{
  for(int i = 0; i < nrows; i++)
    delete [] arrayPtr[i];
  delete [] arrayPtr;
  return;
}


template<class T> void deleteLayerwise(T*** arrayPtr, int nrows, int ncols)
{
  for(int i = 0; i < nrows; i++)
  {
    for(int j = 0; j < ncols; j++)
      delete [] arrayPtr[i][j];
    delete [] arrayPtr[i];
  }
  delete [] arrayPtr;
  return;
}


void initArray(int*** aPtr, int nrows, int ncols)
{
  for(int i = 0; i < nrows; i++)
    for(int j = 0; j < ncols; j++)
      for(int k = 0; k < 4; k++)
        aPtr[i][j][k] = 0;
}


void initArray(int** aPtr, int nrows, int ncols)
{
  for(int i = 0; i < nrows; i++)
    for(int j = 0; j < ncols; j++)
      aPtr[i][j] = 0;
}

void initArray(float*** aPtr, int nrows, int ncols)
{
  for(int i = 0; i < nrows; i++)
    for(int j = 0; j < ncols; j++)
      for(int k = 0; k < 4; k++)
        aPtr[i][j][k] = 0.;
}


void initArray(float** aPtr, int nrows, int ncols)
{
  for(int i = 0; i < nrows; i++)
    for(int j = 0; j < ncols; j++)
      aPtr[i][j] = 0.;
}

void getLayerEffErr(int** muonCount, int*** layerHitCount,
                    float*** layerEffErr, int nrows, int ncols)
{
  for(int l = 0; l < 4; l++)
  {
    for(int i = 0; i < nrows; i++)
    {
      for(int j = 0; j < ncols; j++)
      {
        int k = layerHitCount[i][j][l];
        int n = muonCount[i][j];
        if(muonCount[i][j])
          layerEffErr[i][j][l] = sqrt((double)(k+1)/(n+2)*(k+2)/(n+3)
                                -(double)(k+1)*(k+1)/(n+2)/(n+2));
        else
          layerEffErr[i][j][l] = 0.;
//        cout << layerEffErr[i][j][l] << " ";
      }
//      cout << endl;
    }
//    cout << endl;
  }
//  cout << layerHitCount[1][1][3] << " " << muonCount[1][1] << endl;
}


void getModuleEffErr(float** modEffErr, float*** layerEff, float*** layerEffErr,
      int nrows, int ncols)
{
  for(int i = 0; i < nrows; i++)
  {
    for(int j = 0; j < ncols; j++)
    {
      float D1f, D2f, D3f, D4f;
      float x1, x2, x3, x4;
      float dx1, dx2, dx3, dx4;
      x1 = layerEff[i][j][0];
      dx1 = layerEffErr[i][j][0];
      x2 = layerEff[i][j][1];
      dx2 = layerEffErr[i][j][1];
      x3 = layerEff[i][j][2];
      dx3 = layerEffErr[i][j][2];
      x4 = layerEff[i][j][3];
      dx4 = layerEffErr[i][j][3];
      D1f = x3*x4+x2*x4+x2*x3-3*x2*x3*x4;
      D2f = x1*x3+x3*x4+x1*x4-3*x1*x3*x4;
      D3f = x1*x2+x2*x4+x1*x4-3*x1*x2*x4;
      D4f = x1*x2+x2*x3+x1*x3-3*x1*x2*x3;
      D1f *= dx1;
      D2f *= dx2;
      D3f *= dx3;
      D4f *= dx4;
      modEffErr[i][j] = sqrt(D1f*D1f+D2f*D2f+D3f*D3f+D4f*D4f);
      cout << modEffErr[i][j] << " ";
    }
    cout << endl;
  }
  cout << endl;
}
