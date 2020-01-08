#!/usr/bin/env python


# usage:
#   $ nuwa.py --dbconf 'offline_db' -n 100 -m"StoppedMuon.MuonLookBack"
#     /path/to/root/file/...root

# $ nuwa.py -A 1*s -n 100 -m"StoppedMuon.MuonLookBack" /eliza16/dayabay/data/exp/dayabay/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root

# On heplinux3
# $ nuwa.py -A 1*s -n 100 -m"StoppedMuon.MuonLookBack" /disk1/data/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root | grep MuonLookBack


def configure(argv=[]):
  
  # Parsing:
  from optparse import OptionParser
  parser = OptionParser()
  parser.add_option("-o","--output", action = "store", type = "string", default = "output.root", help = "root file name")
  parser.add_option("-s","--windowstart", action = "store", type = "float", default = 0.5, help = "start of the time window after a muon in microsecond")
  parser.add_option("-e","--windowend", action = "store", type = "float", default = 20, help = "end of the time window after a muon in microsecond")
  (opts,args) = parser.parse_args(args=argv)
  
  # configure RpcGeomInfoSvc
  from DetHelpers.DetHelpersConf import RpcGeomInfoSvc
  rpgisvc = RpcGeomInfoSvc("RpcGeomInfoSvc")
  rpgisvc.StreamItems = [ "/dd/Structure/DayaBay" ]
  
  from Gaudi.Configuration import ApplicationMgr
  myStoppedMuonApp = ApplicationMgr()
  
  from StoppedMuon.StoppedMuonConf import MuonLookBack
  myAlg = MuonLookBack()
  myStoppedMuonApp.TopAlg.append(myAlg)
  myAlg.FileName = opts.output
  myAlg.WindowStart = opts.windowstart
  myAlg.WindowEnd = opts.windowend
  
  return
