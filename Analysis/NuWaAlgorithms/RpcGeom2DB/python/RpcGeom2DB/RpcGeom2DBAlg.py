#!/usr/bin/env python

# usage:
#
# on pdsf:
# $ nuwa.py -A None -n 1 --dbconf=offline_db -m"RpcGeom2DB.RpcGeom2DBAlg" /eliza16/dayabay/data/exp/dayabay/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root

def configure():
  
  # configure RpcGeomInfoSvc
  from DetHelpers.DetHelpersConf import RpcGeomInfoSvc
  rpgisvc = RpcGeomInfoSvc("RpcGeomInfoSvc")
  rpgisvc.StreamItems = [ "/dd/Structure/DayaBay" ]
  
  from Gaudi.Configuration import ApplicationMgr
  rpcGeom2DBApp = ApplicationMgr()
  
  from RpcGeom2DB.RpcGeom2DBConf import RpcGeom2DBAlg
  myAlg = RpcGeom2DBAlg()
  rpcGeom2DBApp.TopAlg.append(myAlg)
  
  return
