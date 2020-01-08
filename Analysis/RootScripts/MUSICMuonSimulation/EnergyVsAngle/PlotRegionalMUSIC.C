void PlotMUSIC()
{
  TFile* inf = new TFile("MUSIC.root");
  TTree* tr = (TTree*)inf->Get("tDYB");
  
  TCanvas* c1 = new TCanvas("c1","c1",800,800);
  c1->Divide(1,3);
  
  c1->cd(1);
  c1->cd(1)->SetLogy();
  TH1F* h1 = new TH1F("h1","region 1",850,0,8500);
  tr->Project("h1","E","cos(theta*3.14/180.)>0.5&&phi>240&&phi<360&&E>20");
  h1->GetXaxis()->SetTitle("energy (GeV)");
  h1->Draw();
  
  c1->cd(2);
  c1->cd(2)->SetLogy();
  TH1F* h2 = new TH1F("h2","region 2",850,0,8500);
  tr->Project("h2","E","cos(theta*3.14/180.)<0.5&&phi>120&&phi<240&&E>20");
  h2->GetXaxis()->SetTitle("energy (GeV)");
  h2->Draw();
  
  c1->cd(3);
  c1->cd(3)->SetLogy();
  TH1F* h3 = new TH1F("h3","region 3",850,0,8500);
  tr->Project("h3","E","cos(theta*3.14/180.)<0.5&&phi>0&&phi<120&&E>20");
  h3->GetXaxis()->SetTitle("energy (GeV)");
  h3->Draw();
}
