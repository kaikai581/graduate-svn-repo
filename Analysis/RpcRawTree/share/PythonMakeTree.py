# 2011.12.26 18:50 Damn! No dictionary for daqRpcStripList() yet! Joy only for
# 20 min!
# 2011.12.26 18:30 found that dictionary for rpcPanelReadouts() is ready in
# NuWa trunk!!
# 2011.12.26 18:00 seems that no indexing available for rpcPanelReadouts() yet

#!/usr/bin/env python
#
# Script for making raw data into root files
#
#
#  Usage:
#   nuwa.py -A None -n -1 -m"PythonMakeTree" daq.NoTag.....data
#
#

# Load DybPython
from DybPython.DybPythonAlg import DybPythonAlg
from GaudiPython import SUCCESS, FAILURE
# import ROOT libraries
from ROOT import gROOT, AddressOf, TTree, TFile

# Define tree structure
gROOT.ProcessLine(\
  "struct RpcRawTree{\
    Bool_t hasTriggers;\
    Int_t triggerTimeSec;\
    Int_t triggerTimeNanoSec;\
    Double_t triggerSpan;\
    Int_t localTriggerNumber;\
    Int_t site;\
    Int_t detector;\
  };")

# Make my own algorithm
class RawToRootAlg(DybPythonAlg):
  "Make Raw into Root Algorithm"
  def __init__(self, name):
    DybPythonAlg.__init__(self, name)
    return

  def initialize(self):
    status = DybPythonAlg.initialize(self)
    if status.isFailure():
      return status
    self.info("initializing")
    from ROOT import RpcRawTree
    trraw = RpcRawTree()
    # Define tree structure

    return SUCCESS

  def execute(self):
#    self.info("executing")
    evt = self.evtSvc()
    readoutHdr = evt["/Event/Readout/ReadoutHeader"]
    if readoutHdr == None:
      self.error("Failed to get current readout header")
      return FAILURE
    
    daqCrate = readoutHdr.daqCrate()
    if daqCrate == None:
      self.info("No daqCrate readout this cycle")
      return SUCCESS
      

    # Trigger Time: Absolute time of trigger for this raw data
    triggerTime = daqCrate.triggerTime()
    # Trigger Time [Seconds]: Trigger time in seconds from some day in 1990
#    self.info("Trigger Time [Seconds part]: "
#              +str( triggerTime.GetSec() ))
    # Trigger Time [Nanoseconds]: Nanoseconds part of trigger time
#    self.info("Trigger Time [Nanoseconds part]: "
#              +str( triggerTime.GetNanoSec() ))
    # Full Trigger Time: Seconds + nanoseconds
    # Warning: When you add this together, it will lose some precision.
#    self.info("Full Trigger Time: "
#              +str( triggerTime.GetSec()
#                    +triggerTime.GetNanoSec()*1.0e-9 ))
    
    readout = daqCrate.asRpcCrate()
    if readout == None:
#      self.info("No rpc readout this cycle")
      return SUCCESS
    else:
      self.info("rpc readout found")
      
#      self.info("Trigger time: " + readout.triggerTime().AsString("2"))
      vecPanel = readout.rpcPanelReadouts()
      for pIdx in range(len(vecPanel)):
        vecStrip = vecPanel[pIdx].daqRpcStripList()
        for sIdx in range(len(vecStrip)):
          self.info('row: ' + vecPanel[pIdx].row())
          self.info('column: ' + vecPanel[pIdx].col())
          self.info('channel id: ' + vecStrip[sIdx].channelId())
        
      
      # Get the detector ID for this trigger
#      detector = daqCrate.detector()
#      self.info("Detector Name: "+detector.detName())
          
      # Trigger Type: This is an integer of the type for this trigger
#      self.info("Trigger Type: "+str( daqCrate.triggerType() ))
      # Event Number: A count of the trigger, according to the DAQ
#      self.info("Event Number: "+str( daqCrate.eventNumber() ))
      
      
      return SUCCESS
    
    return SUCCESS
    
  def finalize(self):
    self.info("finalizing")
    status = DybPythonAlg.finalize(self)
    return status
    
#####  Job Configuration for nuwa.py ########################################

def configure( argv=[] ):
  """ Example of processing raw data """

  # Setup root file for output histograms
  from StatisticsSvc.StatisticsSvcConf import StatisticsSvc
  statsSvc = StatisticsSvc()
  statsSvc.Output = {"file1":"rawDataResult.root"}
  return

def run(app):
  '''
  Configure and add the algorithm to job
  '''
  app.ExtSvc += ["StatisticsSvc"]
  myAlg = RawToRootAlg("MyRawToRootAlg")
  app.addAlgorithm(myAlg)
  pass

