#!/usr/bin/env python

# usage:
#
# on pdsf:
# $ nuwa.py -A None -n 1 --dbconf=offline_db -m"GeomTest.PrintAllAlg" /eliza16/dayabay/data/exp/dayabay/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root
#
# on heplinux3:
# $ nuwa.py -A None -n 0 --dbconf=offline_db -m"GeomTest.PrintAllAlg"

def configure():
  
  # configure RpcGeomInfoSvc
  from DetHelpers.DetHelpersConf import RpcGeomInfoSvc
  rpgisvc = RpcGeomInfoSvc("RpcGeomInfoSvc")
  rpgisvc.StreamItems = [ "/dd/Structure/DayaBay" ]
  
  from Gaudi.Configuration import ApplicationMgr
  printAdPmtApp = ApplicationMgr()
  
  from GeomTest.GeomTestConf import PrintAllAlg
  myAlg = PrintAllAlg()
  printAdPmtApp.TopAlg.append(myAlg)
  
  return
