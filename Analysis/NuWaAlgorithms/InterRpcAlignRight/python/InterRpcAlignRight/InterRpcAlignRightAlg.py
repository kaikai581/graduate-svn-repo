#!/usr/bin/env python

# usage:
#
# on pdsf:
# $ nuwa.py -A 1*s -n 1 --dbconf=offline_db -m"InterRpcAlignRight.InterRpcAlignRightAlg" /eliza16/dayabay/data/exp/dayabay/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root

# On heplinux3
# $ nuwa.py -A 1*s -n 100 -m"InterRpcAlignRight.InterRpcAlignRightAlg -o interrpc.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root" /disk1/data/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root


def configure(argv=[]):

  # Parsing:
  from optparse import OptionParser
  parser = OptionParser()
  parser.add_option("-o","--output", action = "store", type = "string", default = "output.root", help = "root file name")
  (opts,args) = parser.parse_args(args=argv)

  from Gaudi.Configuration import ApplicationMgr
  thisApp = ApplicationMgr()
  
  from InterRpcAlignRight.InterRpcAlignRightConf import InterRpcAlignRightAlg
  myAlg = InterRpcAlignRightAlg()
  thisApp.TopAlg.append(myAlg)
  myAlg.FileName = opts.output
  
  return
