/// ROOT cannot handle 3D vectors directly.
/// It has to be done manually.
/// Run this script after a TFile is opened. Then
/// [0] rpcTree->Draw("mydraw.C")
/// Refer to
/// http://root.cern.ch/phpBB3/viewtopic.php?t=9596&view=previous
unsigned int mydraw() {
   for(unsigned int i = 1; i < mRpcStrip->size(); ++i)
   {
      for(unsigned int j = 0; j < (*mRpcStrip)[i].size(); j++)
        for(unsigned int k = 0; k < (*mRpcStrip)[i][j].size(); k++)
       htemp->Fill((*mRpcStrip)[i][j][k]);
   }
   return (*mRpcStrip)[0][0][0];
}
