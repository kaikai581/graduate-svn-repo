#!/usr/bin/env python


#   Usage:
#   $ nuwa.py -A 1*s -n 100 -m"InterRPC" /disk1/data/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root



# Load DybPython
from DybPython.DybPythonAlg import DybPythonAlg
from GaudiPython import SUCCESS, FAILURE
from GaudiPython import gbl

# Make shortcuts to ROOT classes
TTree = gbl.TTree

# Load standard modules
from array import array

# Algorithm for finding stopped muons
class InterRPC(DybPythonAlg):
  "An algorithm for looking at what happens between RPC triggers"
  def __init__(self, name):
    DybPythonAlg.__init__(self, name)
    self.lastRpcTime = None
    
    return
  
  def initialize(self):
    status = DybPythonAlg.initialize(self)
    if status.isFailure(): return status
    self.info("initializing")
    
    self.tr = TTree("intrpc","AD events between RPC events")
    self.makeBranch()
    self.stats['/file0/event/intrpc'] = self.tr
    return SUCCESS
  
  def execute(self):
    """event loop"""
    evt = self.evtSvc()
    self.info("executing InterRPC")
    
    # load calibration readout tree
    calibHdr = evt["/Event/CalibReadout/CalibReadoutHeader"]
    
    if calibHdr == None:
      self.error("Failed to get current calib readout header")
      return FAILURE
    
    # Access the Readout.  This is the calibrated data from one trigger.
    calibReadout = calibHdr.calibReadout()
    if calibReadout == None:
      self.error("Failed to get calibrated readout from header")
      return FAILURE

    # If this is a RPC event, record the trigger time
    detector = calibHdr.context().GetDetId()
    if detector == 7:
      self.lastRpcTime = calibReadout.triggerTime()
      self.info(str(self.lastRpcTime.GetSec())+"\t"+
                str(self.lastRpcTime.GetNanoSec()))
      return SUCCESS
    elif self.lastRpcTime == None:
      self.info("no RPC event prior to this one")
      return SUCCESS
    elif detector == 1 or detector == 2 or detector == 3 or detector == 4:
      self.info("this is an AD"+str(detector)+" event")
      recHdr = evt["/Event/Rec/AdSimple"]
      recTrigger = recHdr.recTrigger()
      if recTrigger.energyStatus() != 1: return SUCCESS
      self.info("reconstructed energy: "+str(recTrigger.energy())+" MeV")
      dtstamp = calibReadout.triggerTime().CloneAndSubtract(self.lastRpcTime)
      dt = dtstamp.GetSec()*1000000000+dtstamp.GetNanoSec()
      self.info("last RPC time: "+str(self.lastRpcTime.GetSec()*1000000000+self.lastRpcTime.GetNanoSec()))
      self.info("current time: "+str(calibReadout.triggerTime().GetSec()*1000000000+calibReadout.triggerTime().GetNanoSec()))
      self.info("time to the last RPC trigger: "+str(dtstamp.GetSec())+"s\t"+str(dtstamp.GetNanoSec())+"ns")
      self.info("or "+str(dt)+"ns")
      self.dt[0] = dt
      self.e[0] = recTrigger.energy()
      self.detector[0] = detector
      self.isFlasher[0] = self.flasherTest()
      self.tr.Fill()
      return SUCCESS
    elif detector == 5:
      self.info("this is a WPI event")
      return SUCCESS
    else:
      self.info("this is a WPO event")
      return SUCCESS
    
    return SUCCESS


  def finalize(self):
    self.info("finalizing")
    status = DybPythonAlg.finalize(self)
    return status



  def flasherTest(self):
    evt = self.evtSvc()
    calibStats = evt["/Event/Data/CalibStats"]
    if calibStats == None:
      self.debug("No calibrated statistics!")
      return -1
    Quadrant = calibStats.get('Quadrant').value()
    MaxQ = calibStats.get('MaxQ').value()
    flasherScale = Quadrant*Quadrant+(MaxQ/.45)*(MaxQ/.45)
    if flasherScale < 1:
      return 0
    return 1



  def makeBranch(self):
    self.dt = array('L', [0])
    self.e  = array('d', [0.])
    self.detector = array('i', [0])
    self.isFlasher = array('i', [0])
    
    self.tr.Branch('dt', self.dt, 'dt/l')
    self.tr.Branch('e', self.e, 'e/D')
    self.tr.Branch('detector', self.detector, "detector/I")
    self.tr.Branch('isFlasher', self.isFlasher, "isFlasher/I")


#####  Job Configuration for nuwa.py ###########################################

def configure(argv=[]):
  # Process module options
  import sys, getopt
  opts,args = getopt.getopt(argv,"o:")
  statsOutput = "interRPCTree.root"
  initializeDataSvc = True
  for opt,arg in opts:
    if opt == "-o":
      statsOutput = arg
  if statsOutput != "None":
    from StatisticsSvc.StatisticsSvcConf import StatisticsSvc
    statsSvc = StatisticsSvc()
    statsSvc.Output = {"file0":statsOutput}
  return



def run(app):
  app.ExtSvc += ["StatisticsSvc"]
  myAlg = InterRPC("InterRPC")
  app.addAlgorithm(myAlg)
  pass
