#!/usr/bin/env python


# usage:
#   $ nuwa.py --dbconf 'offline_db' -n 100 -m"Event2DB_v04.Event2DB_v04"
#     /path/to/root/file/...root

# $ nuwa.py -A 1*s -n 10 -m"Event2DB_v04.Event2DB_v041Alg" /eliza16/dayabay/data/exp/dayabay/2011/p12e/Neutrino/1224/recon.Neutrino.0021221.Physics.EH1-Merged.P12E-P._0001.root

# On heplinux3
# $ nuwa.py -A 1*s -n 100 -m"Event2DB_v04.Event2DB_v041Alg" /disk1/data/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root | grep AESTest


def configure():
  
  from Gaudi.Configuration import ApplicationMgr
  event2DB_v041App = ApplicationMgr()
  
  from Event2DB_v04.Event2DB_v04Conf import Event2DB_v041Alg
  myAlg = Event2DB_v041Alg()
  event2DB_v041App.TopAlg.append(myAlg)
  
  return
