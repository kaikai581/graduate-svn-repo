#!/usr/bin/env python

# usage:
#
# on pdsf:
# $ nuwa.py -A None -n 1 --dbconf=offline_db -m"GeomTest.PrintAdPmtAlg" /eliza16/dayabay/data/exp/dayabay/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root
#
# on heplinux3:
# $ nuwa.py -A None -n 0 --dbconf=offline_db -m"GeomTest.PrintAdPmtAlg" /disk1/data/2011/p12e/Neutrino/1224/recon.Neutrino.0021221.Physics.EH1-Merged.P12E-P._0001.root

def configure():
  
  from Gaudi.Configuration import ApplicationMgr
  printAdPmtApp = ApplicationMgr()
  
  from GeomTest.GeomTestConf import PrintAdPmtAlg
  myAlg = PrintAdPmtAlg()
  printAdPmtApp.TopAlg.append(myAlg)
  
  return
