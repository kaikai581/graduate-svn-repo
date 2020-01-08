#include <cmath>
#include <fstream>
#include <iostream>
#include "TChain.h"
#include "TF1.h"
#include "TH1F.h"
#include "TROOT.h"


using namespace std;

void Interarrival(string inf = "", int row = 1, int col = 1, string leaf = "dtInSec4Fold")
{
  if(inf == "")
  {
    cout << "Please input file names" << endl;
    return;
  }

  TChain aCh("rpcTree");
  aCh.Add(inf.c_str());
  aCh.Draw(Form("%s>>%s",leaf.c_str(),leaf.c_str()), Form("%s>0&&mRpcRow==%d&&mRpcColumn==%d", leaf.c_str(), row, col));

  TH1F* hDt = (TH1F*)gDirectory->Get(Form("%s",leaf.c_str()));
  if(!hDt) return;
  hDt->SetTitle("");
  hDt->GetXaxis()->SetTitle("#Deltat (s)");
  hDt->GetYaxis()->SetTitle("count");
  hDt->Draw();

  TF1 *ffit = (TF1*)gROOT->GetFunction("expo");

  hDt->Fit(ffit);
  ofstream ofs("rate_error.txt");
  ofs << abs(ffit->GetParameter(1)) << " " << ffit->GetParError(1) << endl;
  ofs.close();
}
