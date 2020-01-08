#include <fstream>
#include "TFile.h"
#include "TTree.h"



char *fnlist[] = {"/disk1/NuWa/external/data/0.0/Muon/mountain_DYB",
                  "/disk1/NuWa/external/data/0.0/Muon/mountain_LA",
                  "/disk1/NuWa/external/data/0.0/Muon/mountain_Far_80m"};

void MUSIC2Tree()
{
  TFile* outf = new TFile("MUSIC.root","recreate");
  TTree* outt[3];
  
  outt[0] = new TTree("tDYB", "Daya Bay site");
  outt[1] = new TTree("tLA", "Ling Ao site");
  outt[2] = new TTree("tFar", "Far site");
  
  double Eb4[3], thetab4[3], phib4[3];
  double E[3], theta[3], phi[3];
  
  for(int i = 0; i < 3; i++)
  {
    outt[i]->Branch("Eb4",&Eb4[i],"Eb4/D");
    outt[i]->Branch("thetab4",&thetab4[i],"thetab4/D");
    outt[i]->Branch("phib4",&phib4[i],"phib4/D");
    outt[i]->Branch("E",&E[i],"E/D");
    outt[i]->Branch("theta",&theta[i],"theta/D");
    outt[i]->Branch("phi",&phi[i],"phi/D");
  }
  for(int fnum = 0; fnum < 3; fnum++)
  {
    ifstream inf(fnlist[fnum]);
    
    string infline;
    /// flush the first 6 lines
    for(int i = 0; i < 6; i++) getline(inf, infline);
    
    int nTest = 0;
    int nRead = 0;
    while(inf >> infline)
    {
      if(!(nRead%6)) nTest++;
      
      int nCol = nRead%6;
      
      switch (nCol) {
        case 0:
          E[fnum] = atof(infline.c_str());
          break;
        case 1:
          theta[fnum] = atof(infline.c_str());
          break;
        case 2:
          phi[fnum] = atof(infline.c_str());
          break;
        case 3:
          Eb4[fnum] = atof(infline.c_str());
          break;
        case 4:
          thetab4[fnum] = atof(infline.c_str());
          break;
        case 5:
          phib4[fnum] = atof(infline.c_str());
          outt[fnum]->Fill();
          break;
      }
      
      nRead++;
    }
  }
  
  outt[0]->Write();
  outt[1]->Write();
  outt[2]->Write();
  outf->Close();
}
