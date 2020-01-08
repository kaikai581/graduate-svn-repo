#!/usr/bin/env python


# 2012/7/31:
# This script ran into problem that the RpcRec related variables don't seem
# to have python bindings. I'm reverting to C++ again.


#   Usage:
#   $ nuwa.py -A 0.2*s -n 100 -m"StoppedMuons" /eliza16/dayabay/data/exp/dayabay/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root



# Load DybPython
from DybPython.DybPythonAlg import DybPythonAlg
from GaudiPython import SUCCESS, FAILURE
from GaudiPython import gbl

# Make shortcuts to ROOT classes
TTree = gbl.TTree

# Load standard modules
from array import array

# Algorithm for finding stopped muons
class StoppedMuonsByAD(DybPythonAlg):
  "Find stopped muons self tagged by ADs"
  def __init__(self, name):
    DybPythonAlg.__init__(self, name)
    
    return
  
  def initialize(self):
    status = DybPythonAlg.initialize(self)
    if status.isFailure(): return status
    self.info("initializing")
    
    # prepare the output tree
    self.smTree = TTree("stoppedMuonTree", "stopped muon tree");
    
    # member variables for tree leaves
    self.hitAD1 = array('i', [0])
    self.hitAD2 = array('i', [0])
    self.hitAD3 = array('i', [0])
    self.hitAD4 = array('i', [0])
    self.hitIWS = array('i', [0])
    self.hitOWS = array('i', [0])
    self.hitRPC = array('i', [0])
    self.rpcX   = array('d', [0])
    self.rpcY   = array('d', [0])
    self.rpcZ   = array('d', [0])
    
    # assign branches to the output tree
    self.smTree.Branch('hitAD1', self.hitAD1,'hitAD1/I')
    self.smTree.Branch('hitAD2', self.hitAD2,'hitAD2/I')
    self.smTree.Branch('hitAD3', self.hitAD3,'hitAD3/I')
    self.smTree.Branch('hitAD4', self.hitAD4,'hitAD4/I')
    self.smTree.Branch('hitIWS', self.hitIWS,'hitIWS/I')
    self.smTree.Branch('hitOWS', self.hitOWS,'hitOWS/I')
    self.smTree.Branch('hitRPC', self.hitRPC,'hitRPC/I')
    
    return SUCCESS
  
  def execute(self):
    """event loop"""
    evt = self.evtSvc()
    self.info("executing StoppedMuonsByAD")
    
    # assign trees to variables
    spallData = evt["/Event/Data/Physics/Spallation"]
    rpcRec = evt["/Event/Rec/RpcSimple"]
    
    if spallData == None:
      self.error("No spallation data this cycle")
      return SUCCESS
    else:
      self.info("Spallation data found this cycle")
      self.info("Start processing the event")
      
      # start assigning values to tree leaves
      self.hitAD1 = spallData.get('hitAD1').value()
      self.info("hitAD1="+str(self.hitAD1))
      
      self.hitRPC = spallData.get('hitRPC').value()
      if self.hitRPC == 1:
        self.rpcX = rpcRec.clusters[0].x()
        self.info("rpcX="+str(self.rpcX))
      
      return SUCCESS
    
    return SUCCESS
  
  def finalize(self):
    self.info("finalizing")
    status = DybPythonAlg.finalize(self)
    return status



#####  Job Configuration for nuwa.py ###########################################

def configure(argv=[]):
  return



def run(app):
  app.ExtSvc += ["StatisticsSvc"]
  myAlg = StoppedMuonsByAD("MyStoppedMuonsByAD")
  app.addAlgorithm(myAlg)
  pass
