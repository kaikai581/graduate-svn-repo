#!/usr/bin/env python


# usage:
#   $ nuwa.py --dbconf 'offline_db' -n 100 -m"StoppedMuon.AESTest"
#     /path/to/root/file/...root

# $ nuwa.py -A 1*s -n 100 -m"StoppedMuon.AESTest" /eliza16/dayabay/data/exp/dayabay/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root

# On heplinux3
# $ nuwa.py -A 1*s -n 100 -m"StoppedMuon.AESTest" /disk1/data/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root | grep AESTest


def configure():
  
  from Gaudi.Configuration import ApplicationMgr
  myStoppedMuonApp = ApplicationMgr()
  
  from StoppedMuon.StoppedMuonConf import AESTest
  myAlg = AESTest()
  myStoppedMuonApp.TopAlg.append(myAlg)
  
  return
