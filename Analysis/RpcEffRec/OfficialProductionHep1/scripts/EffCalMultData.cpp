#include <climits>
#include <fstream>
#include <iostream>
#include "TCanvas.h"
#include "TFile.h"
#include "TH2I.h"
#include "TPaveLabel.h"
#include "TROOT.h"
#include "TStyle.h"

bool fexists(const char *filename)
{
  ifstream ifile(filename);
  return ifile;
}

/// example of input file name
/// p12b run 21358
/// ../file/*21358*P12B*.root
void EffCalMultData(string infn = "test.root")
{
  gROOT->ProcessLine(Form(".! ls %s > inflist", infn.c_str()));
  ifstream ifs("inflist", ifstream::in);
  string infpn;
  int ***totLayerHitCount = 0, **totPassMuonCount = 0;
  int **threeHitCount = 0, **fourHitCount = 0;
  float ***layerEff = 0, **modEff = 0;
  float **hitRatio = 0, **effRatio = 0;
  float **eff3Hit = 0, **eff4Hit = 0;
  int maxHitCount = 0;
  
  int minHitCount = INT_MAX;
  int ncols = 0, nrows = 0;
  TH2I *layerCount[4];
  TH2I *muonCount = 0;
  TH2I *h3Hit = 0;
  TH2I *h4Hit = 0;
  
  while(ifs >> infpn)
  {
    cout << "opening file: " << endl;
    cout << infpn << endl;
    TFile *inf = new TFile(infpn.c_str(), "read");
    for(int i = 0; i < 4; i++)
      layerCount[i] = (TH2I*)inf->Get(Form("l%d",i));
    muonCount = (TH2I*)inf->Get("m");
    h3Hit = (TH2I*)inf->Get("h3h");
    h4Hit = (TH2I*)inf->Get("h4h");
    
    ncols = muonCount->GetNbinsX();
    nrows = muonCount->GetNbinsY();
    int ***layerHitCount = 0, **passMuonCount = 0;
    
    
    /// results memory allocation and initialization
    if(!totLayerHitCount&&!totPassMuonCount&&!layerEff&&!modEff
      &&!threeHitCount&&!fourHitCount&&!hitRatio&&!effRatio
      &&!eff3Hit&&!eff4Hit)
    {
      totLayerHitCount = new int** [nrows];
      layerEff = new float** [nrows];
      totPassMuonCount = new int* [nrows];
      modEff = new float* [nrows];
      hitRatio = new float* [nrows];
      effRatio = new float* [nrows];
      eff3Hit = new float* [nrows];
      eff4Hit = new float* [nrows];
      threeHitCount = new int* [nrows];
      fourHitCount = new int* [nrows];
      
      for(int i = 0; i < nrows; i++)
      {
        totLayerHitCount[i] = new int* [ncols];
        layerEff[i] = new float* [ncols];
        totPassMuonCount[i] = new int [ncols];
        modEff[i] = new float [ncols];
        hitRatio[i] = new float [ncols];
        effRatio[i] = new float [ncols];
        eff3Hit[i] = new float [ncols];
        eff4Hit[i] = new float [ncols];
        threeHitCount[i] = new int [ncols];
        fourHitCount[i] = new int [ncols];
        for(int j = 0; j < ncols; j++)
        {
          totPassMuonCount[i][j] = 0;
          modEff[i][j] = 0.;
          threeHitCount[i][j] = 0;
          fourHitCount[i][j] = 0;
          hitRatio[i][j] = 0.;
          effRatio[i][j] = 0.;
          eff3Hit[i][j] = 0.;
          eff4Hit[i][j] = 0.;
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
    
    if(!layerHitCount||!passMuonCount)
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
        passMuonCount[i][j] = muonCount->GetBinContent(j+1, i+1);

        totPassMuonCount[i][j] += passMuonCount[i][j];
        if(totPassMuonCount[i][j] > maxHitCount)
          maxHitCount = totPassMuonCount[i][j];
        if(h3Hit)
          threeHitCount[i][j] += h3Hit->GetBinContent(j+1, i+1);
        if(h4Hit)
          fourHitCount[i][j] += h4Hit->GetBinContent(j+1, i+1);
      }


    /// add up layer hit count
    for(int k = 0; k < 4; k++)
    {
      for(int i = 0; i < nrows; i++)
      {
        for(int j = 0; j < ncols; j++)
        {
          layerHitCount[i][j][k] = layerCount[k]->GetBinContent(j+1, i+1);
          totLayerHitCount[i][j][k] += layerHitCount[i][j][k];
/*          if(passMuonCount[i][j])
            layerEff[i][j][k] = (float)layerHitCount[i][j][k]
                                /passMuonCount[i][j];
          else
            layerEff[i][j][k] = 0.;
          cout << layerEff[i][j][k] << " ";
          if(layerEff[i][j][k] < layerMinEff && layerEff[i][j][k] > 0.)
            layerMinEff = layerEff[i][j][k];*/
          if(totLayerHitCount[i][j][k] < minHitCount &&
            totLayerHitCount[i][j][k] > 0)
            minHitCount = totLayerHitCount[i][j][k];
        }
//        cout << endl;
      }
//      cout << endl;
    }
    
    /// release memory
    for(int i = 0; i < nrows; i++)
    {
      for(int j = 0; j < ncols; j++)
        delete [] layerHitCount[i][j];
      delete [] layerHitCount[i];
      delete [] passMuonCount[i];
    }
    delete layerHitCount;
    delete passMuonCount;
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
      cout << endl;
    }
    cout << endl;
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
        eff3Hit[i][j] += threeProd;
        fourProd *= layerEff[i][j][k];
      }
      modEff[i][j] += fourProd;
      eff4Hit[i][j] += fourProd;
      if(eff3Hit[i][j] > 0.)
        effRatio[i][j] = eff4Hit[i][j]/eff3Hit[i][j];
      else
        effRatio[i][j] = 0.;
      if(fourHitCount[i][j])
        hitRatio[i][j] = (float)fourHitCount[i][j]/threeHitCount[i][j];
      else
        hitRatio[i][j] = 0.;
      cout << modEff[i][j] << " ";
      if(modEff[i][j] < minEff && modEff[i][j] > 0.)
        minEff = modEff[i][j];
      /// make value in percentage
      modEff[i][j] *= 100.;
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
  
  /// make value in percentage
  for(int k = 0; k < 4; k++)
    for(int i = 0; i < nrows; i++)
      for(int j = 0; j < ncols; j++)
        layerEff[i][j][k] *= 100.;
  
  
  /// draw module efficiency
  TH2F *hModEff = new TH2F("eff", "calculated module efficiency",
                            ncols, 0, ncols, nrows, 0, nrows);
  for(int i = 0; i < nrows; i++)
    for(int j = 0; j < ncols; j++)
      hModEff->SetBinContent(j+1, i+1, modEff[i][j]);
  hModEff->SetStats(0);
  hModEff->SetMinimum(((float)((int)(minEff*10))/10)*100.);
  hModEff->SetMaximum(1.*100.);
  hModEff->GetXaxis()->SetTitle("column");
  hModEff->GetYaxis()->SetTitle("row");
  TCanvas *c1 = new TCanvas("c1");
  gStyle->SetPaintTextFormat("3.1f%%");
  hModEff->Draw("colz,text");
  TPaveLabel *ae = new TPaveLabel(5.4, .2, 8.6, .8,
                        Form("hall average: %3.1f%%", avgEff));
  ae->SetBorderSize(0);
  ae->SetFillColor(kWhite);
  ae->SetShadowColor(kWhite);
  ae->Draw();
  c1->SaveAs("ModEff.pdf");
  
  
  /// draw layer efficiency
  TH2F *hLayerEff[4];
  TCanvas *c2 = new TCanvas("c2", "c2", 600, 600);
  c2->Draw();
  c2->Divide(2,2);
  for(int k = 0; k < 4; k++)
  {
    hLayerEff[k] = new TH2F(Form("l%deff", k),
                            Form("layer efficiency for layer %d", k),
                            ncols, 0, ncols, nrows, 0, nrows);
    for(int i = 0; i < nrows; i++)
      for(int j = 0; j < ncols; j++)
        hLayerEff[k]->SetBinContent(j+1, i+1, layerEff[i][j][k]);
    hLayerEff[k]->SetStats(0);
    hLayerEff[k]->SetMinimum(((float)((int)(layerMinEff*10))/10)*100.);
    hLayerEff[k]->SetMaximum(1.*100.);
    hLayerEff[k]->GetXaxis()->SetTitle("column");
    hLayerEff[k]->GetYaxis()->SetTitle("row");
    c2->cd(k+1);
    hLayerEff[k]->Draw("colz,text");
  }
  c2->SaveAs("LayerEff.pdf");
  gStyle->SetPaintTextFormat("g");
  
  /// draw muon counts
  TCanvas *c3   = new TCanvas("c3", "c3", 900, 600); 
  TPad    *lPad = new TPad("lPad", "lPad", 0, 0, .667, 1);
  TPad    *rPad = new TPad("rPad", "rPad", .667, .5, 1, 1);
  lPad->Draw();
  rPad->Draw();
  lPad->Divide(2, 2);
  for(int k = 0; k < 4; k++)
  {
    layerCount[k] = new TH2I(Form("l%d", k),
                            Form("hit count for layer %d", k),
                            ncols, 0, ncols, nrows, 0, nrows);
    for(int i = 0; i < nrows; i++)
      for(int j = 0; j < ncols; j++)
        layerCount[k]->SetBinContent(j+1, i+1, totLayerHitCount[i][j][k]);
    lPad->cd(k+1);
    layerCount[k]->SetStats(0);
    layerCount[k]->GetXaxis()->SetTitle("column");
    layerCount[k]->GetYaxis()->SetTitle("row");
    layerCount[k]->SetMinimum(minHitCount);
    layerCount[k]->SetMaximum(maxHitCount);
    layerCount[k]->Draw("colz,text");
  }
  rPad->cd();
  muonCount = new TH2I("m", "number of muons",
                      ncols, 0, ncols, nrows, 0, nrows);
  for(int i = 0; i < nrows; i++)
    for(int j = 0; j < ncols; j++)
      muonCount->SetBinContent(j+1, i+1, totPassMuonCount[i][j]);
  muonCount->SetStats(0);
  muonCount->GetXaxis()->SetTitle("column");
  muonCount->GetYaxis()->SetTitle("row");
  muonCount->SetMinimum(minHitCount);
  muonCount->SetMaximum(maxHitCount);
  muonCount->Draw("colz,text");
  c3->SaveAs("LayerCount.pdf");
  
  
  /// draw 3 hit to 4 hit ratio
  gStyle->SetPaintTextFormat("2.1f");
  TH2F *hEffRatio = new TH2F("h3to4",
                              "3 hit efficiency to 4 hit efficiency ratio",
                              ncols, 0, ncols, nrows, 0, nrows);
  TH2F *hHitRatio = new TH2F("h3hto4h", "3 hit readout to 4 hit readout ratio",
                              ncols, 0, ncols, nrows, 0, nrows);
  TCanvas *c4 = new TCanvas("c4", "c4", 1200, 600);
  c4->Draw();
  c4->Divide(2,1);
  
  for(int i = 0; i < nrows; i++)
    for(int j = 0; j < ncols; j++)
      hEffRatio->SetBinContent(j+1, i+1, effRatio[i][j]);
  hEffRatio->SetStats(0);
  hEffRatio->GetXaxis()->SetTitle("column");
  hEffRatio->GetYaxis()->SetTitle("row");
  c4->cd(1);
  hEffRatio->Draw("colz,text");
  for(int i = 0; i < nrows; i++)
    for(int j = 0; j < ncols; j++)
      hHitRatio->SetBinContent(j+1, i+1, hitRatio[i][j]);
  hHitRatio->SetStats(0);
  hHitRatio->GetXaxis()->SetTitle("column");
  hHitRatio->GetYaxis()->SetTitle("row");
  c4->cd(2);
  hHitRatio->Draw("colz,text");

  c4->SaveAs("HitRatio.pdf");
  gStyle->SetPaintTextFormat("g");
  
  
    
  
  if(fexists("inflist"))
    gROOT->ProcessLine(".! rm -f inflist");

}
