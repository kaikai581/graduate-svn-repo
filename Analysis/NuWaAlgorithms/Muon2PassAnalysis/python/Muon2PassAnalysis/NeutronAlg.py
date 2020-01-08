#!/usr/bin/env python


# usage:
#   $ nuwa.py --dbconf 'offline_db' -n 100 -m"Muon2PassAnalysis.NeutronAlg"
#     /path/to/root/file/...root

# $ nuwa.py -A 1*s -n 100 -m"Muon2PassAnalysis.NeutronAlg -r21358 -f1" /eliza16/dayabay/data/exp/dayabay/2012/p12e/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0001.root

# On heplinux3
# $ nuwa.py -A 1*s -n 100 -m"Muon2PassAnalysis.NeutronAlg -r21358 -f1" /disk1/data/2012/p12e/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0001.root


def configure(argv=[]):
  
  # Parsing:
  from optparse import OptionParser
  parser = OptionParser()
  parser.add_option("-r","--runnum", action = "store", type = "string", default = "0021358", help = "run number of the file")
  parser.add_option("-f","--filenum", action = "store", type = "string", default = "0001", help = "file number of the file")
  (opts,args) = parser.parse_args(args=argv)
  
  from Gaudi.Configuration import ApplicationMgr
  muon2PassAnalysisApp = ApplicationMgr()
  
  from Muon2PassAnalysis.Muon2PassAnalysisConf import NeutronAlg
  myAlg = NeutronAlg()
  muon2PassAnalysisApp.TopAlg.append(myAlg)
  myAlg.RunNum = opts.runnum
  myAlg.FileNum = opts.filenum
  
  return
