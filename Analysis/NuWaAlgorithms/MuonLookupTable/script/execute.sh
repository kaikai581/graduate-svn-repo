#!/bin/sh

. ~/Scripts/nuwaenv
. ~/Work/Analysis/NuWaAlgorithms/PhysicsProcesses/StoppedMuon/cmt/setup.sh

time nuwa.py -A 1*s -n 0 -m"MuonLookupTable.ReconValidAlg -i/eliza16/dayabay/data/exp/dayabay/2012/p12e/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12E-P._0001.root" /eliza16/dayabay/data/exp/dayabay/2012/p12e/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12E-P._0001.root
