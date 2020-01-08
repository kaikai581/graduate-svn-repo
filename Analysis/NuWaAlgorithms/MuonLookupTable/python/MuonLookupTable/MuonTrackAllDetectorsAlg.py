#!/usr/bin/env python


# usage:
#   $ nuwa.py --dbconf 'offline_db' -n 100 -m"MuonLookupTable.MuonTrackAllDetectorsAlg"
#     /path/to/root/file/...root

# $ nuwa.py -A 1*s -n 100 -m"MuonLookupTable.MuonTrackAllDetectorsAlg -i/eliza16/dayabay/data/exp/dayabay/2012/p12e/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12E-P._0001.root" /eliza16/dayabay/data/exp/dayabay/2012/p12e/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12E-P._0001.root

# On heplinux3
# $ nuwa.py -A 1*s -n 1000 -m"MuonLookupTable.MuonTrackAllDetectorsAlg -i/disk1/data/2011/p12e/Neutrino/1224/recon.Neutrino.0021221.Physics.EH1-Merged.P12E-P._0001.root" /disk1/data/2011/p12e/Neutrino/1224/recon.Neutrino.0021221.Physics.EH1-Merged.P12E-P._0001.root


def configure(argv=[]):
  
  # Parsing:
  from optparse import OptionParser
  parser = OptionParser()
  parser.add_option("-i","--input", action = "store", type = "string", default = "input.root", help = "input root file name")
  parser.add_option("-o","--output", action = "store", type = "string", default = "output.root", help = "output root file name")
  #parser.add_option("-s","--windowstart", action = "store", type = "float", default = 20., help = "start of the time window after a muon in microsecond")
  #parser.add_option("-e","--windowend", action = "store", type = "float", default = 1000., help = "end of the time window after a muon in microsecond")
  (opts,args) = parser.parse_args(args=argv)
  
  from Gaudi.Configuration import ApplicationMgr
  MuonLookupTableApp = ApplicationMgr()
  
  from MuonLookupTable.MuonLookupTableConf import MuonTrackAllDetectorsAlg
  myAlg = MuonTrackAllDetectorsAlg()
  MuonLookupTableApp.TopAlg.append(myAlg)
  myAlg.InFileName = opts.input
  myAlg.OutFileName = opts.output
  #myAlg.WindowStart = opts.windowstart
  #myAlg.WindowEnd = opts.windowend
  
  return
