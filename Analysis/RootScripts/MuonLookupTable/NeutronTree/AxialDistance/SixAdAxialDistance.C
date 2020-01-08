void SixAdAxialDistance()
{
  TCanvas* c1 = new TCanvas("c1", "c1", 900, 600);
  c1->Divide(3,2);
  int numpad = 0;
  for(int hall = 1; hall <= 3; hall++)
  {
    int maxad;
    if(hall == 1) maxad = 2;
    if(hall == 2) maxad = 1;
    if(hall == 3) maxad = 3;
    for(int ad = 1; ad <= maxad; ad++)
    {
      c1->cd(++numpad);
      gROOT->ProcessLine(Form(".x OneAdAxialDistance.C(%d,%d)", hall, ad));
    }
  }
}
