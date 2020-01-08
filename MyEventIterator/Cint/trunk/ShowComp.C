void ShowComp()
{
	c1=new TCanvas("c1");
	gROOT->SetStyle("Plain");
	gROOT->ForceStyle();
	gPad->UseCurrentStyle();
	TFile *f1 = new TFile("nhitscan_0_1576.root");
	TFile *f2 = new TFile("nhitscan_0_5764.root");

	TH1F *h100_1;
	f1->GetObject("hHit64",h100_1);
	TH1F *h100_2;
	f2->GetObject("hHit64",h100_2);
	h100_1->Draw();
	h100_1->SetLineColor(kBlue);
	h100_2->Draw("same");
	h100_2->SetLineColor(kRed);
	h100_1->SetStats(0);
	TLegend *l1=new TLegend(.75,.8,.95,.95);
	l1->AddEntry(h100_1,"run#1576");
	l1->AddEntry(h100_2,"run#5764");
	l1->Draw();
	c1->SaveAs("100ns.ps");

	TH1F *h200_1, *h200_2;
	f1->GetObject("hHit128",h200_1);
	f2->GetObject("hHit128",h200_2);
	h200_1->SetLineColor(kBlue);
	h200_2->SetLineColor(kRed);
	h200_1->SetStats(0);
	h200_1->Draw();
	h200_2->Draw("same");
	TLegend *l2=new TLegend(.75,.8,.95,.95);
	l2->AddEntry(h200_1,"run#1576");
	l2->AddEntry(h200_2,"run#5764");
	l2->Draw();
	c1->SaveAs("200ns.ps");
}
