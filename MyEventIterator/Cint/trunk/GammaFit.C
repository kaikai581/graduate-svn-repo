/******************************************************************************
Fit of the inter arrival time of muon events (t1-t0), (t2-t0), etc.
They obey gamma distribution, c.f. Nucl. Phys. B, 370 (1992) 432.
October 2010 @ Daya Bay by Shih-Kai Lin.
******************************************************************************/
#define MAXORDER 4


void GammaFit(char *fn, int gammaOrder)
{
	TFile *f1=new TFile(fn);
	TH1F *hIA[4];
	int i;
	TString tempString;

	if(gammaOrder<1)
	{
		cout << "The order of gamma distribution starts from 1!" << endl;
		return;
	}
	if(gammaOrder>MAXORDER)
	{
		cout << "The order of gamma distribution can't exceed " << MAXORDER << "!" << endl;
		return;
	}

	for(i = 0; i < MAXORDER; i++)
	{
		tempString = Form("hInterTime%d",i);
		hIA[i] = (TH1F*)f1->Get(tempString);
	}

	//definition of gamma distribution; gamma order starts from 1
	tempString = Form("[0]*[1]*(TMath::Power([1]*x,%d-1))*exp(-1*[1]*x)/(TMath::Factorial(%d-1))",gammaOrder,gammaOrder);
	TF1 *fun1 = new TF1("fun1",tempString);
	fun1->SetParameters(3,1000);

	hIA[gammaOrder-1]->Fit(fun1);
}
