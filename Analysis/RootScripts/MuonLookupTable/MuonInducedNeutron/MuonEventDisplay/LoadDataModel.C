void LoadDataModel()
{
  gSystem->AddIncludePath("-I$PERCALIBREADOUTEVENTROOT");
  gSystem->AddIncludePath("-I$PERBASEEVENTROOT");
  gSystem->AddIncludePath("-I$ROOTIOSVCROOT");
  gSystem->AddIncludePath("-I$CONTEXTROOT");
  gSystem->AddIncludePath("-I$CONVENTIONSROOT");
  gROOT->ProcessLine(".x $ROOTIOTESTROOT/share/load.C");
}
