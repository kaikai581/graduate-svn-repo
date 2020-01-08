#include <vector>

void DrawEvent(int evtNum=0, int modIdx=0, int runNum=21358, int fileNum=2, string fp="../EnrichedTrees/2012/p12b/Neutrino/0101")
{
  gROOT->ProcessLine(Form(".! ls %s/*%d*%d* >flist", fp.c_str(), runNum, fileNum));
  string vfpn;
  ifstream flist("flist");
  
  if(!(flist>>vfpn))
  {
    cerr << "file is not found" << endl;
    return;
  }
  
  gROOT->ProcessLine(".L ../include/vecdict.h+");
  TFile* fif = new TFile(vfpn.c_str());
  TTree* tRpc = (TTree*)fif->Get("rpcTree");
  std::vector<std::vector<unsigned int> >* mRpcLayer;
  std::vector<std::vector<std::vector<unsigned int> > >* mRpcStrip;
  tRpc->SetBranchAddress("mRpcLayer", &mRpcLayer);
  tRpc->SetBranchAddress("mRpcStrip", &mRpcStrip);
  
  tRpc->GetEntry(evtNum);
  
  cout << mRpcLayer->size() << " module(s) in this readout" << endl;
  
  
  TH2I* hRpcVis = new TH2I("hRpcVis", "", 8, 1, 9, 8, 1, 9);
  for(int i = 1; i < 9; i++)
    for(int j = 1; j < 9; j++)
    {
      hRpcVis->SetBinContent(i,j,0);
    }
  for(unsigned int l = 0; l < mRpcLayer->at(modIdx).size(); l++)
  {
    if(!((mRpcLayer->at(modIdx)[l]-1)%3)) // x strips
    {
      for(unsigned int s = 0; s < mRpcStrip->at(modIdx)[l].size(); s++)
      {
        for(int i = 0; i < 9; i++)
        {
          int stripId = mRpcStrip->at(modIdx)[l][s];
          int binCont = hRpcVis->GetBinContent(stripId,i);
          hRpcVis->SetBinContent(stripId,i,++binCont);
        }
      }
    }
    else // y strips
    {
      for(unsigned int s = 0; s < mRpcStrip->at(modIdx)[l].size(); s++)
      {
        for(int i = 0; i < 9; i++)
        {
          int stripId = mRpcStrip->at(modIdx)[l][s];
          int binCont = hRpcVis->GetBinContent(i,stripId);
          hRpcVis->SetBinContent(i,stripId,++binCont);
        }
      }
    }
  }
  hRpcVis->SetStats(0);
  TCanvas *c1=new TCanvas("c1","c1",1000,500);
  c1->Divide(2,1);
  c1->cd(1);
  gPad->SetGridx(1);
  gPad->SetGridy(1);
  hRpcVis->Draw("coltext");
  
  
  c1->cd(2)->Divide(2,2);
  TH2I* hLayer[4];
  for(int i = 0; i < 4; i++)
  {
    c1->cd(2)->cd(i+1);
    if(i%2) gPad->SetGridy(1);
    else gPad->SetGridx(1);
    hLayer[i] = new TH2I(Form("l%d",i+1),Form("layer%d",i+1),8,1,9,8,1,9);
    hLayer[i]->SetStats(0);
  }
  for(int k = 0; k < 4; k++)
  {
    for(int i = 1; i < 9; i++)
      for(int j = 0; j < 9; j++)
      {
        hLayer[k]->SetBinContent(i,j,0);
      }
  }
  
  
  for(unsigned int l = 0; l < mRpcLayer->at(modIdx).size(); l++)
  {
    int layerId = mRpcLayer->at(modIdx)[l];
    if(!((layerId-1)%3)) // x strips
    {
      for(unsigned int s = 0; s < mRpcStrip->at(modIdx)[l].size(); s++)
      {
        for(int i = 0; i < 9; i++)
        {
          int stripId = mRpcStrip->at(modIdx)[l][s];
          int binCont = hLayer[layerId-1]->GetBinContent(stripId,i);
          hLayer[layerId-1]->SetBinContent(stripId,i,++binCont);
        }
      }
    }
    else // y strips
    {
      for(unsigned int s = 0; s < mRpcStrip->at(modIdx)[l].size(); s++)
      {
        for(int i = 0; i < 9; i++)
        {
          int stripId = mRpcStrip->at(modIdx)[l][s];
          int binCont = hLayer[layerId-1]->GetBinContent(i,stripId);
          hLayer[layerId-1]->SetBinContent(i,stripId,++binCont);
        }
      }
    }
  }
  

  c1->cd(2)->cd(1);
  hLayer[0]->Draw("col");
  c1->cd(2)->cd(2);
  hLayer[1]->Draw("col");
  c1->cd(2)->cd(3);
  hLayer[3]->Draw("col");
  c1->cd(2)->cd(4);
  hLayer[2]->Draw("col");
    
  
  
  
  flist.close();
  gROOT->ProcessLine(".! rm -f flist");
}
