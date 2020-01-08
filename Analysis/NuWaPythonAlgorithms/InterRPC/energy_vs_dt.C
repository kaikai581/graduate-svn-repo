void energy_vs_dt()
{
  TFile* inf = new TFile("interRPCTree.root");
  
  TTree* tr = (TTree*)inf->Get("/stats/event/intrpc");
  
  TH2F* h1 = new TH2F("h1", "h1", 1000,0,66000000,1000,0,5000);
  

  tr->Project("h1","e:dt","detector==1");
  h1->GetXaxis()->SetTitle("time since last RPC trigger (ns)");
  h1->GetYaxis()->SetTitle("energy (MeV)");
  
  TH2F* h2 = new TH2F("h2", "h2", 1000,0,66000000,1000,0,5000);
  tr->Project("h2","e:dt","detector==1&&isFlasher==1");
  h2->GetXaxis()->SetTitle("time since last RPC trigger (ns)");
  h2->SetMarkerColor(kRed);
  

  h1->Draw();
  h2->Draw("same");
  c1->SaveAs("energy_inter_rpc_events.gif");
  
  
  TH2F* h3 = new TH2F("h3","h3",1000,0,66000000,100,0,100);
  TH2F* h4 = new TH2F("h4","h4",1000,0,66000000,100,0,100);
  
  
  tr->Project("h3","e:dt","detector==1&&e<100");
  tr->Project("h4","e:dt","detector==1&&e<100&&isFlasher==1");
  h3->GetXaxis()->SetTitle("time since last RPC trigger (ns)");
  h3->GetYaxis()->SetTitle("energy (MeV)");
  h4->SetMarkerColor(kRed);
  
  TCanvas* c100 = new TCanvas("c100");
  h3->Draw();
  h4->Draw("same");
  c100->SaveAs("energy_inter_rpc_events_low_energy.gif");
}
