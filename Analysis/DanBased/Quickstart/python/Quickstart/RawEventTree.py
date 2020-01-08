#!/usr/bin/env python
#
# Example script to make a ROOT tree containing raw ADC/TDC PMT data.
#
# Usage:
#  nuwa.py -A None -n -1 -m"Quickstart.RawEventTree" daq.NoTag.....data
# 

# Load GaudiPython
from DybPython.DybPythonAlg import DybPythonAlg
from GaudiPython import SUCCESS, FAILURE
from GaudiPython import gbl, loaddict
from DybPython.Util import irange

# Make shortcuts to ROOT classes
TTree = gbl.TTree

Detector = gbl.DayaBay.Detector
Site = gbl.Site
DetectorId = gbl.DetectorId
ServiceMode = gbl.ServiceMode
AdPmtSensor = gbl.DayaBay.AdPmtSensor

from array import array
import math

class TesToTreeAlg(DybPythonAlg):
    "Dump TES contents to ROOT tree"
    def __init__(self,name):
        DybPythonAlg.__init__(self,name)
        # List of headers in output tree
        self.HeaderList = ["Readout"]
        return

    def initialize(self):
        status = DybPythonAlg.initialize(self)
        if status.isFailure(): return status
        self.info("initialize")

        # Cable Service: This service provides a mapping between
        # electronics channels and pmts in the detector.
        self.cableSvc = self.svc('ICableSvc','StaticCableSvc')
        if self.cableSvc == None:
            self.error("Failed to get StaticCableSvc")
            return FAILURE
        
        self.firstEntry = True
        
        self.tesTree= TTree("eventTree","Event data converted to ROOT Tree")

        self.initReadoutHeader()

        self.stats['/file0/event/eventTree'] = self.tesTree
        return SUCCESS

    def initReadoutHeader(self):
        # ReadoutEvent Data
        self.hasRo = array('i',[0])
        self.roTimeSec = array('i',[0])
        self.roTimeNanoSec = array('i',[0])
        self.hasReadout = array('i',[0])
        self.roSite = array('i',[0])
        self.roDetector = array('i',[0])
        self.roTriggerNumber = array('i',[0])
        self.roTriggerTimeSec = array('i',[0])
        self.roTriggerTimeNanoSec = array('i',[0])
        self.nRoChannels = array('i',[0])
        self.nRoTdcs = array('i',[0])
        self.nRoAdcs = array('i',[0])
        self.maxRoTdcs = 4096
        self.roBoard = array('i',self.maxRoTdcs*[0])
        self.roConnector = array('i',self.maxRoTdcs*[0])
        self.roTdc = array('i',self.maxRoTdcs*[0])
        self.maxRoAdcs = 4096
        self.roAdcClock = array('i',self.maxRoAdcs*[0])
        self.roAdc = array('i',self.maxRoAdcs*[0])
        self.roPed = array('i',self.maxRoAdcs*[0])
        self.roAdcGain = array('i',self.maxRoAdcs*[0])
        self.roRing = array('i',self.maxRoTdcs*[0])
        self.roColumn = array('i',self.maxRoTdcs*[0])
        
        self.tesTree.Branch('hasReadoutHeader',self.hasRo,'hasReadoutHeader/I')
        self.tesTree.Branch('roTimeSec',self.roTimeSec,'roTimeSec/I')
        self.tesTree.Branch('roTimeNanoSec',self.roTimeNanoSec,
                            'roTimeNanoSec/I')
        self.tesTree.Branch('hasReadout',self.hasReadout,'hasReadout/I')
        self.tesTree.Branch('site',self.roSite,'site/I')
        self.tesTree.Branch('detector',self.roDetector,'detector/I')
        self.tesTree.Branch('triggerNumber',self.roTriggerNumber,
                            'triggerNumber/I')
        self.tesTree.Branch('triggerTimeSec',self.roTriggerTimeSec,
                            'triggerTimeSec/I')
        self.tesTree.Branch('triggerTimeNanoSec',self.roTriggerTimeNanoSec,
                            'triggerTimeNanoSec/I')
        self.tesTree.Branch('nChannels',self.nRoChannels,'nChannels/I')
        self.tesTree.Branch('nTdcs',self.nRoTdcs,'nTdcs/I')
        self.tesTree.Branch('nAdcs',self.nRoAdcs,'nAdcs/I')
        self.tesTree.Branch('board',self.roBoard,'board[nTdcs]/I')
        self.tesTree.Branch('connector',self.roConnector,
                            'connector[nTdcs]/I')
        self.tesTree.Branch('tdc',self.roTdc,'tdc[nTdcs]/I')
        self.tesTree.Branch('adcClock',self.roAdcClock,
                            'adcClock[nAdcs]/I')
        self.tesTree.Branch('adcGain',self.roAdcGain,
                            'adcGain[nAdcs]/I')
        self.tesTree.Branch('adc',self.roAdc,'adc[nAdcs]/I')
        self.tesTree.Branch('ped',self.roPed,'ped[nAdcs]/I')
        self.tesTree.Branch('ring',self.roRing,'ring[nTdcs]/I')
        self.tesTree.Branch('column',self.roColumn,
                            'column[nTdcs]/I')

    def execute(self):
        self.info("executing")

        self.writeReadoutHeader()

        self.tesTree.Fill()
        self.firstEntry = False
        return SUCCESS

    def writeReadoutHeader(self):
        # ReadoutEvent Data
        self.hasRo[0] = 0
        self.roTimeSec[0] = 0
        self.roTimeNanoSec[0] = 0
        self.hasReadout[0] = 0
        self.roSite[0] = 0
        self.roDetector[0] = 0
        self.roTriggerNumber[0] = 0
        self.roTriggerTimeSec[0] = 0
        self.roTriggerTimeNanoSec[0] = 0
        self.nRoChannels[0] = 0
        self.nRoTdcs[0] = 0
        self.nRoAdcs[0] = 0

        evt = self.evtSvc()
        # ReadoutEvent Data
        roHdr = evt["/Event/Readout/ReadoutHeader"]
        if roHdr != None:
            self.hasRo[0] = 1
            self.roTimeSec[0] = roHdr.timeStamp().GetSec()
            self.roTimeNanoSec[0] = roHdr.timeStamp().GetNanoSec()
            svcMode = ServiceMode( roHdr.context(), 0 )
            readout = roHdr.readout()
            if readout != None:
                self.hasReadout[0] = 1
                self.roSite[0] = readout.detector().site()
                detId = readout.detector().detectorId()
                self.roDetector[0] = detId
                self.roTriggerNumber[0] = readout.triggerNumber()
                trigTime = readout.triggerTime()
                self.roTriggerTimeSec[0] = trigTime.GetSec()
                self.roTriggerTimeNanoSec[0] = trigTime.GetNanoSec()
                if (detId == DetectorId.kAD1 or
                    detId == DetectorId.kAD2 or
                    detId == DetectorId.kAD3 or
                    detId == DetectorId.kAD4):
                    nRoChannels = 0
                    nRoTdcs = 0
                    nRoAdcs = 0
                    for channelPair in readout.channelReadout():
                        channel = channelPair.second
                        chanId = channel.channelId()
                        for tdcIdx in range( channel.size() ):
                            self.roBoard[nRoTdcs] = chanId.board()
                            self.roConnector[nRoTdcs] = chanId.connector()
                            self.roTdc[nRoTdcs] = channel.tdc( tdcIdx )
                            self.roAdc[nRoAdcs] = channel.adc( tdcIdx )
                            self.roPed[nRoAdcs] = channel.pedestal( tdcIdx )
                            self.roAdcGain[nRoAdcs] = channel.adcRange( tdcIdx )
                            self.roAdcClock[nRoAdcs] = channel.adcCycle( tdcIdx )
                            # PMT Ring and Column
                            pmtId = self.cableSvc.adPmtSensor(chanId, svcMode)
                            self.roRing[nRoTdcs] = pmtId.ring()
                            self.roColumn[nRoTdcs] = pmtId.column()
                            nRoTdcs += 1
                            nRoAdcs += 1
                        nRoChannels += 1
                    self.nRoChannels[0] = nRoChannels
                    self.nRoTdcs[0] = nRoTdcs
                    self.nRoAdcs[0] = nRoAdcs
        return
        
    def finalize(self):        
        self.info("finalizing")
        status = DybPythonAlg.finalize(self)
        return status

###############################################################################

def configure(argv = []):
    # Process module options
    import sys, getopt
    opts,args = getopt.getopt(argv,"o:")
    statsOutput = "eventTree.root"
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
    '''
    Configure and add an algorithm to job
    '''
    app.ExtSvc += ["StatisticsSvc","StaticCableSvc"]
    tesToTreeAlg = TesToTreeAlg("TesToTreeAlg")
    app.addAlgorithm(tesToTreeAlg)
    pass
