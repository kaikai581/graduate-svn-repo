#!/usr/bin/env python


# usage:
#   $ nuwa.py --dbconf 'offline_db' -n 100 -m"HighSchool2013Summer.MuonRedefinitionAlg"
#     /path/to/root/file/...root

# $ nuwa.py -A 1*s -n 100 -m"HighSchool2013Summer.MuonRedefinitionAlg -i/eliza16/dayabay/data/exp/dayabay/2012/p12e/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12E-P._0001.root" /eliza16/dayabay/data/exp/dayabay/2012/p12e/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12E-P._0001.root

# On heplinux3
# $ nuwa.py -A 1*s -n 1000 -m"HighSchool2013Summer.MuonRedefinitionAlg -i/disk1/data/2011/p12e/Neutrino/1224/recon.Neutrino.0021221.Physics.EH1-Merged.P12E-P._0001.root" /disk1/data/2011/p12e/Neutrino/1224/recon.Neutrino.0021221.Physics.EH1-Merged.P12E-P._0001.root


def configure(argv=[]):
  
  # Parsing:
  from optparse import OptionParser
  parser = OptionParser()
  parser.add_option("-i","--input", action = "store", type = "string", default = "input.root", help = "input root file name")
  (opts,args) = parser.parse_args(args=argv)
  
  from Gaudi.Configuration import ApplicationMgr
  HighSchool2013SummerApp = ApplicationMgr()
  
  from HighSchool2013Summer.HighSchool2013SummerConf import MuonRedefinitionAlg
  myAlg = MuonRedefinitionAlg()
  HighSchool2013SummerApp.TopAlg.append(myAlg)
  myAlg.InFileName = opts.input
  
  return
