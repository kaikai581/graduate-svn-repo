void PlotMUSIC()
{
  TFile* inf = new TFile("MUSIC.root");
  TTree* tDYB = (TTree*)inf->Get("tDYB");
  TTree* tLA = (TTree*)inf->Get("tLA");
  TTree* tFar = (TTree*)inf->Get("tFar");
  
  TCanvas* c1 = new TCanvas("c1","c1",800,800);
  c1->Divide(1,3);
  
  c1->cd(1);
  c1->cd(1)->SetLogy();
  TH1F* h1 = new TH1F("h1","Hall 1",850,0,8500);
  tDYB->Project("h1","E");
  h1->GetXaxis()->SetTitle("energy (GeV)");
  h1->Draw();
  
  c1->cd(2);
  c1->cd(2)->SetLogy();
  TH1F* h2 = new TH1F("h2","Hall 2",850,0,8500);
  tLA->Project("h2","E");
  h2->GetXaxis()->SetTitle("energy (GeV)");
  h2->Draw();
  
  c1->cd(3);
  c1->cd(3)->SetLogy();
  TH1F* h3 = new TH1F("h3","Hall 3",850,0,8500);
  tFar->Project("h3","E");
  h3->GetXaxis()->SetTitle("energy (GeV)");
  h3->Draw();
}
