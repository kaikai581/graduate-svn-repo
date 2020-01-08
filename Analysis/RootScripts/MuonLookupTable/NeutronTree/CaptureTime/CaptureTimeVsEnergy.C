string path = "/global/project/projectdirs/dayabay/scratch/sklin/Analysis/StandAlone/MuonInducedNeutrons/output/";

void CaptureTimeVsEnergy(int hall, int ad)
{
  stringstream pathname;
  pathname << path << "EH" << hall << "*";
  
  TChain ch("trInt");
  ch.Add(pathname.str().c_str());
  
  TCanvas *c1 = new TCanvas("c1", "c1", 500, 500);
  
  TH2F* h2 = new TH2F("h2", "time since muon vs. energy", 100, 0, 30, 100, 0, 200);
  ch.Project("h2", "dtNeu*1e6:enNeu", Form("adNeu==%d&&enNeu<100", ad));
  h2->GetXaxis()->SetTitle("energy (MeV)");
  h2->GetXaxis()->CenterTitle();
  h2->GetYaxis()->SetTitle("time since muon (#mus)");
  h2->GetYaxis()->CenterTitle();
  h2->Draw("col");
  gPad->SetLogz(1);
}
