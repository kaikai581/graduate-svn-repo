string path = "/global/project/projectdirs/dayabay/scratch/sklin/Analysis/StandAlone/MuonInducedNeutrons/output/";

void OneAdCaptureTime(int hall, int ad)
{
  stringstream pathname;
  pathname << path << "EH" << hall << "*";
  
  TChain ch("trInt");
  ch.Add(pathname.str().c_str());
  
  ch.Draw("dtNeu*1e6", Form("adNeu==%d&&enNeu>6&&enNeu<12&&inCylNeu&&dtNeu<300e-6", ad));
}
