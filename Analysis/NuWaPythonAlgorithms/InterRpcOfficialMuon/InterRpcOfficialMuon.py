#!/usr/bin/env python


# 2012/9/12:
# This script is not capable of processing multiple AD triggers in a tagged
# muon event.


#   Usage:
#   $ nuwa.py -A 1*s -n 100 -m"InterRpcOfficialMuon -o intrpc.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root" /eliza16/dayabay/data/exp/dayabay/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root



# Load DybPython
from DybPython.DybPythonAlg import DybPythonAlg
from GaudiPython import SUCCESS, FAILURE
from GaudiPython import gbl

# Shortcuts
Trigger = gbl.DayaBay.Trigger

# Make shortcuts to ROOT classes
TTree = gbl.TTree

# Load standard modules
from array import array

# Algorithm for finding stopped muons
class InterRpcOfficialMuon(DybPythonAlg):
  "Find stopped muons self tagged by ADs"
  def __init__(self, name):
    DybPythonAlg.__init__(self, name)
    
    return
  
  def initialize(self):
    status = DybPythonAlg.initialize(self)
    if status.isFailure(): return status
    self.info("initializing")
    
    # prepare the output tree
    self.tr = TTree("InterRpcTree", "AD events since the last RPC trigger");
    
    # member variables for tree leaves
    self.dt = array('l', [0])
    self.e  = array('d', [0.])
    self.detector = array('i', [0])
    self.isFlasher = array('i', [0])
    self.iws = array('i', [0])
    self.ows = array('i', [0])
    self.trigType = array('i', [0])
    
    # assign branches to the output tree
    self.tr.Branch('dt', self.dt, 'dt/L')
    self.tr.Branch('e', self.e, 'e/D')
    self.tr.Branch('detector', self.detector, "detector/I")
    self.tr.Branch('isFlasher', self.isFlasher, "isFlasher/I")
    self.tr.Branch('hitIWS', self.iws, 'hitIWS/I')
    self.tr.Branch('hitOWS', self.ows, 'hitOWS/I')
    self.tr.Branch('triggerType', self.trigType, 'triggerType/I')
    
    self.stats['/file0/event/intrpc'] = self.tr
    
    self.evtcnt = 0
    
    return SUCCESS
  
  def execute(self):
    """event loop"""
    evt = self.evtSvc()
    self.debug("executing InterRpcOfficialMuon")
    if self.evtcnt % 1000 == 0:
      self.info(str(self.evtcnt/1000*1000)+" events done")
    self.evtcnt += 1
    
    # assign trees to variables
    self.spallData = evt["/Event/Data/Physics/Spallation"]
    
    self.AesAd = self.getAES("/Event/Rec/AdSimple")
    self.AesCalib = self.getAES("/Event/CalibReadout/CalibReadoutHeader")
    self.AesCalibStats = self.getAES("/Event/Data/CalibStats")
    
    if self.spallData == None:
      self.debug("No spallation data this cycle")
      return SUCCESS
    else:
      self.debug("Spallation data found this cycle")
      self.debug("Start processing the event")
      
      # only deal with events with both AD and RPC triggers
      hitAD = {}
      hitAD[1] = self.spallData.get('hitAD1').value()
      hitAD[2] = self.spallData.get('hitAD2').value()
      hitAD[3] = self.spallData.get('hitAD3').value()
      hitAD[4] = self.spallData.get('hitAD4').value()
      self.hitIWS = self.spallData.get('hitIWS').value()
      self.hitOWS = self.spallData.get('hitOWS').value()
      hitRPC = self.spallData.get('hitRPC').value()
      if hitRPC != 1:
        self.debug("No RPC trigger. Ignore.")
        return SUCCESS
      if hitAD[1] != 1 and hitAD[2] != 1 and hitAD[3] != 1 and hitAD[4] != 1:
        self.debug("No AD trigger. Ignore.")
        return SUCCESS
      
      # find RPC trigger time
      self.tnRPC = self.spallData.get('triggerNumber_RPC').value()
      self.trigerTimeRPC = self.getTriggerTimeRPC()
      self.debug("RPC trigger time: "+self.trigerTimeRPC.AsString())
      
      # fill WP related variables
      self.iws[0] = self.hitIWS
      self.ows[0] = self.hitOWS
      
      # fill AD related variables
      for k in range(1,5):
        if hitAD[k] == 1:
          self.fillAdVars(k)
          self.fillTree(k)
      
      return SUCCESS
    
    return SUCCESS
  
  def finalize(self):
    self.info("finalizing")
    status = DybPythonAlg.finalize(self)
    return status


  def getTriggerTimeRPC(self):
    for calibHdr in self.AesCalib:
      if calibHdr == None:
        self.error("Failed to get calib readout header from AES")
        return FAILURE
      
      # Access the Readout. This is the calibrated data from one trigger.
      calibReadout = calibHdr.calibReadout()
      if calibReadout == None:
        self.error("Failed to get calibrated readout from header")
        return FAILURE
      
      # A trigger is uniquely identified with its trigger number and detector ID
      if calibReadout.triggerNumber() == self.tnRPC and calibHdr.context().GetDetId() == 7:
        return calibReadout.triggerTime()
      
    return 0
  
  
  def fillAdVars(self, k):
    tnAD = self.spallData.get('triggerNumber_AD'+str(k)).value()
    
    # retrieve reconstructed data
    for recHdr in self.AesAd:
      if recHdr == None:
        self.error("Failed to get reconstructed readout header from AES")
        return FAILURE

      # Access the Readout. This is the reconstructed data from one trigger.
      recTrigger = recHdr.recTrigger()
      if recTrigger == None:
        self.error("Failed to get reconstructed readout from header")
        return FAILURE

      if recTrigger.triggerNumber() == tnAD and recTrigger.detector().detectorId() == k:
        dtstamp = recTrigger.triggerTime().CloneAndSubtract(self.trigerTimeRPC)
        self.dt[0] = -dtstamp.GetSec()*1000000000+dtstamp.GetNanoSec()
        self.e[0] = recTrigger.energy()
        self.detector[0] = k
        self.trigType[0] = recTrigger.triggerType()
        self.debug("#######################IWS: "+str(self.iws[0]))
        self.debug("#######################OWS: "+str(self.ows[0]))
        self.debug("###########DT##############"+str(self.dt[0]))
        break

    # flash identification
    for calibStats in self.AesCalibStats:
      if calibStats == None:
        self.debug("No calibrated statistics in AES!")
        self.isFlasher[0] = -1
      else:
        if calibStats.get('triggerNumber').value() == tnAD and calibStats.context().GetDetId() == k:
          Quadrant = calibStats.get('Quadrant').value()
          MaxQ = calibStats.get('MaxQ').value()
          flasherScale = Quadrant*Quadrant+(MaxQ/.45)*(MaxQ/.45)
          if flasherScale < 1:
            self.isFlasher[0] = 0
          else:
            self.isFlasher[0] = 1
          self.debug("########FLASHER############"+str(self.isFlasher[0]))
          break
    
    return


  def fillTree(self, k):
    self.tr.Fill()
    return


#####  Job Configuration for nuwa.py ###########################################

def configure(argv=[]):
  # Process module options
  import sys, getopt
  opts,args = getopt.getopt(argv,"o:")
  statsOutput = "eventTree.root"
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
  myAlg = InterRpcOfficialMuon("MyInterRpcOfficialMuon")
  app.addAlgorithm(myAlg)
  pass
