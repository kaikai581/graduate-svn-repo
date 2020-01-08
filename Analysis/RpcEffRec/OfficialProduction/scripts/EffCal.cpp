#include <climits>

void EffCal(string infn = "test.root")
{
  cout << "opening file: " << endl;
  cout << infn << endl;
  TFile *inf = new TFile(infn.c_str(), "read");
  TH2I *layerCount[4];
  for(int i = 0; i < 4; i++)
    layerCount[i] = (TH2I*)inf->Get(Form("l%d",i));
  TH2I *muonCount = (TH2I*)inf->Get("m");
  
  int ncols = muonCount->GetNbinsX();
  int nrows = muonCount->GetNbinsY();
  int ***layerHitCount = 0, **passMuonCount = 0;
  float ***layerEff = 0, **modEff = 0;
  
  if(!layerHitCount||!passMuonCount)
  {
    /// row runs from 0..7 for near sites; 0..10 for far site
    /// column runs from 1..9
    
    layerHitCount = new int** [nrows];
    layerEff = new float** [nrows];
    passMuonCount = new int* [nrows];
    modEff = new float* [nrows];
    
    for(int i = 0; i < nrows; i++)
    {
      layerHitCount[i] = new int* [ncols];
      layerEff[i] = new float* [ncols];
      passMuonCount[i] = new int [ncols];
      modEff[i] = new float [ncols];
      for(int j = 0; j < ncols; j++)
      {
        layerHitCount[i][j] = new int [4];
        layerEff[i][j] = new float [4];
      }
    }
    
    
    int maxHitCount = 0;
    for(int i = 0; i < nrows; i++)
      for(int j = 0; j < ncols; j++)
      {
        passMuonCount[i][j] = muonCount->GetBinContent(j+1, i+1);

        modEff[i][j] = 0.;
        if(passMuonCount[i][j] > maxHitCount)
          maxHitCount = passMuonCount[i][j];
      }


    /// calculate layer efficiency
    float layerMinEff = 1.;
    int minHitCount = INT_MAX;
    for(int k = 0; k < 4; k++)
    {
      for(int i = 0; i < nrows; i++)
      {
        for(int j = 0; j < ncols; j++)
        {
          layerHitCount[i][j][k] = layerCount[k]->GetBinContent(j+1, i+1);
          if(passMuonCount[i][j])
            layerEff[i][j][k] = (float)layerHitCount[i][j][k]
                                /passMuonCount[i][j];
          else
            layerEff[i][j][k] = 0.;
          cout << layerEff[i][j][k] << " ";
          if(layerEff[i][j][k] < layerMinEff && layerEff[i][j][k] > 0.)
            layerMinEff = layerEff[i][j][k];
          if(layerHitCount[i][j][k] < minHitCount && layerHitCount[i][j][k] > 0)
            minHitCount = layerHitCount[i][j][k];
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
          fourProd *= layerEff[i][j][k];
        }
        modEff[i][j] += fourProd;
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
    c1->SaveAs("ModEff.eps");
    
    
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
    c2->SaveAs("LayerEff.eps");
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
      lPad->cd(k+1);
      layerCount[k]->SetStats(0);
      layerCount[k]->GetXaxis()->SetTitle("column");
      layerCount[k]->GetYaxis()->SetTitle("row");
      layerCount[k]->SetMinimum(minHitCount);
      layerCount[k]->SetMaximum(maxHitCount);
      layerCount[k]->Draw("colz,text");
    }
    rPad->cd();
    muonCount->SetStats(0);
    muonCount->GetXaxis()->SetTitle("column");
    muonCount->GetYaxis()->SetTitle("row");
    muonCount->SetMinimum(minHitCount);
    muonCount->SetMaximum(maxHitCount);
    muonCount->Draw("colz,text");
    c3->SaveAs("LayerCount.eps");
    
  }
}
