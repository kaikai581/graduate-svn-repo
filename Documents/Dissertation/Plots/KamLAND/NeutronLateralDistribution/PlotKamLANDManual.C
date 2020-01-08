void PlotKamLANDManual()
{
  ifstream inf("kamland_manual.txt");
  string tmpstr;
  /// flush the first line
  getline(inf, tmpstr);
  
  int readctr = 0;
  vector<double> vecx, vecy;
  while(inf >> tmpstr)
  {
    if(readctr%2 == 0) vecx.push_back(atof(tmpstr.c_str()));
    if(readctr%2 == 1) vecy.push_back(atof(tmpstr.c_str()));
    readctr++;
  }
  inf.close();
  
  const int arrsize = vecx.size();
  int binx[arrsize];
  double y[arrsize];
  for(unsigned int i = 0; i < vecx.size(); i++)
  {
    binx[i] = vecx[i]/10;
    y[i] = vecy[i];
    cout << binx[i] << " " << y[i] << endl;
  }
  

  TFile* f1 = new TFile("kamland_manual.root","recreate");
  TH1F* h1 = new TH1F("h1","KamLAND neutron lateral distribution",150,0,15000);
  for(unsigned int i = 0; i < vecx.size(); i++)
    h1->SetBinContent(binx[i]+1,y[i]);
  h1->GetXaxis()->SetTitle("lateral distance (mm)");
  h1->GetXaxis()->CenterTitle();
  h1->GetYaxis()->SetTitle("events/100mm");
  h1->GetYaxis()->CenterTitle();
  h1->SetMaximum(5000);
  h1->SetMinimum(0.1);
  h1->Write();
  
  TH1F* h2 = new TH1F("h2","KamLAND neutron lateral distribution",10,0,1000);
  for(unsigned int i = 0; i < 10; i++)
    h2->SetBinContent(binx[i]+1,y[i]);
  h2->SetMinimum(0.1);
  h2->GetXaxis()->SetTitle("lateral distance (mm)");
  h2->GetXaxis()->CenterTitle();
  h2->GetYaxis()->SetTitle("events/100mm");
  h2->GetYaxis()->CenterTitle();
  /// fit 2D Gaussian from [0,1000]
  TF1* fun2 = new TF1("fun2","[0]*x*exp(-x*x/2/[1]/[1])",0,1000);
  fun2->SetParameters(1000,500);
  h2->Fit(fun2,"N");
  
  double p0 = fun2->GetParameter(0);
  double p1 = fun2->GetParameter(1);
  

  TF1* fun1 = new TF1("fun1",Form("%f*x*exp(-x*x/2/%f/%f)",p0,p1,p1),0,15000);
  fun1->Write();
  h2->Write();
  fun2->Write();
  f1->Close();
  
}
