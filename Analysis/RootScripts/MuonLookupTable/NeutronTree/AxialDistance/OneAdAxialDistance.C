string path = "/global/project/projectdirs/dayabay/scratch/sklin/Analysis/StandAlone/MuonInducedNeutrons/output/";

void OneAdAxialDistance(int hall, int ad)
{
  stringstream pathname;
  pathname << path << "EH" << hall << "*";
  
  TChain ch("trNeu");
  ch.Add(pathname.str().c_str());
  
  TH1F* h1 = new TH1F("h1", Form("EH%d AD%d neutron z distribution", hall, ad), 60, -.6, .6);
  ch.Project("h1", "zAxScaled", Form("adNeu==%d&&inCylNeu", ad));
  h1->GetXaxis()->SetTitle("relative z");
  h1->GetXaxis()->CenterTitle();
  h1->Draw();
}
