#!/usr/bin/env python


# usage:
#   $ nuwa.py --dbconf 'offline_db' -n 100 -m"MuonLookupTable.PrintPoolRpcRecAlg"
#     /path/to/root/file/...root

# $ nuwa.py -A 1*s -n 100 -m"MuonLookupTable.PrintPoolRpcRecAlg" /eliza16/dayabay/data/exp/dayabay/2012/p12e/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12E-P._0001.root

# On heplinux3
# $ nuwa.py -A 1*s -n 100 -m"MuonLookupTable.PrintPoolRpcRecAlg" /disk1/data/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root | grep AESTest


def configure():
  
  from Gaudi.Configuration import ApplicationMgr
  MuonLookupTableApp = ApplicationMgr()
  
  from MuonLookupTable.MuonLookupTableConf import PrintPoolRpcRecAlg
  myAlg = PrintPoolRpcRecAlg()
  MuonLookupTableApp.TopAlg.append(myAlg)
  
  return
