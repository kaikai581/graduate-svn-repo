#!/usr/bin/env python


# usage:
#   $ nuwa.py --dbconf 'offline_db' -n 100 -m"Muon2DB.Muon2DBAlg"
#     /path/to/root/file/...root

# $ nuwa.py -A 1*s -n 100 -m"Muon2DB.Muon2DBAlg" /eliza16/dayabay/data/exp/dayabay/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root

# On heplinux3
# $ nuwa.py -A 1*s -n 100 -m"Muon2DB.Muon2DBAlg" /disk1/data/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root | grep AESTest


def configure():
  
  from Gaudi.Configuration import ApplicationMgr
  muon2DBApp = ApplicationMgr()
  
  from Muon2DB.Muon2DBConf import Muon2DBAlg
  myAlg = Muon2DBAlg()
  muon2DBApp.TopAlg.append(myAlg)
  
  return
