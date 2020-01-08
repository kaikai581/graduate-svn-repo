#!/usr/bin/env python


#   Usage:
#   $ nuwa.py -A 0.2*s -n 100 -m"NuWaAlgDemo" /eliza16/dayabay/data/exp/dayabay/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root



# Load DybPython
from DybPython.DybPythonAlg import DybPythonAlg
from GaudiPython import SUCCESS, FAILURE
from GaudiPython import gbl

# Make shortcuts to ROOT classes
TTree = gbl.TTree

# Load standard modules
from array import array

# Algorithm for finding stopped muons
class NuWaAlgDemo(DybPythonAlg):
  "NuWa demo algorithm"
  def __init__(self, name):
    DybPythonAlg.__init__(self, name)
    
    return
  
  def initialize(self):
    status = DybPythonAlg.initialize(self)
    if status.isFailure(): return status
    self.info("initializing")
    
    return SUCCESS
  
  def execute(self):
    """event loop"""
    evt = self.evtSvc()
    self.info("executing NuWaAlgDemo")
    
    # load spallation tree event
    spallData = evt["/Event/Data/Physics/Spallation"]
    
    if spallData == None:
      self.error("No spallation data this cycle")
      return SUCCESS
    else:
      self.info("Spallation data found this cycle")
      self.info("Start processing the event")
      
      # start assigning values to tree leaves
      self.info("hitAD1="+str(spallData.get('hitAD1').value()))
      
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
  myAlg = NuWaAlgDemo("MyNuWaAlgDemo")
  app.addAlgorithm(myAlg)
  pass
