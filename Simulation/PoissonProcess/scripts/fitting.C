void fitting()
{
  TFile *tif = new TFile("../results.root");
  TH1F *hDtIneff = (TH1F*)tif->Get("hDtIneff");
  hDtIneff->Draw();

  TF1 *fa1 = new TF1("fa1","[0]*[1]*exp(-[0]*x)");
  fa1->SetLineColor(kGreen);
  fa1->SetParameters(.7, 160000);
  hDtIneff->Fit("fa1");
}
