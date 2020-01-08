#!/usr/bin/env python


# usage:
#   $ nuwa.py --dbconf 'offline_db' -n 100 -m"MuonLookupTable.ReconValidAlg"
#     /path/to/root/file/...root

# $ nuwa.py -A 1*s -n 100 -m"MuonLookupTable.ReconValidAlg -i/eliza16/dayabay/data/exp/dayabay/2012/p12e/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12E-P._0001.root" /eliza16/dayabay/data/exp/dayabay/2012/p12e/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12E-P._0001.root

# On heplinux3
# $ nuwa.py -A 1*s -n 100 -m"MuonLookupTable.ReconValidAlg" /disk1/data/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root | grep AESTest


def configure(argv=[]):
  
  # Parsing:
  from optparse import OptionParser
  parser = OptionParser()
  parser.add_option("-i","--input", action = "store", type = "string", default = "input.root", help = "input root file name")
  parser.add_option("-s","--windowstart", action = "store", type = "float", default = 20., help = "start of the time window after a muon in microsecond")
  parser.add_option("-e","--windowend", action = "store", type = "float", default = 1000., help = "end of the time window after a muon in microsecond")
  (opts,args) = parser.parse_args(args=argv)
  
  from Gaudi.Configuration import ApplicationMgr
  MuonLookupTableApp = ApplicationMgr()
  
  from MuonLookupTable.MuonLookupTableConf import ReconValidAlg
  myAlg = ReconValidAlg()
  MuonLookupTableApp.TopAlg.append(myAlg)
  myAlg.InFileName = opts.input
  myAlg.WindowStart = opts.windowstart
  myAlg.WindowEnd = opts.windowend
  
  return
