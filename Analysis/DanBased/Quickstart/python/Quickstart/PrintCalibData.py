#!/usr/bin/env python
#
# Example script for printing raw data to the terminal, and makes one histogram
#
# This script contains comments describing the contents of the raw data. 
#
#  Usage:
#   nuwa.py -A None -n -1 -m"Calibrate" -m"Quickstart.PrintCalibData" daq...data
#
#

# Load DybPython
from DybPython.DybPythonAlg import DybPythonAlg
from GaudiPython import SUCCESS, FAILURE
from GaudiPython import gbl
from DybPython.Util import irange
import GaudiKernel.SystemOfUnits as units

# Make shortcuts to any ROOT classes you want to use
TH1F = gbl.TH1F
TimeStamp = gbl.TimeStamp
AdPmtSensor = gbl.DayaBay.AdPmtSensor
Detector = gbl.DayaBay.Detector

#change default ROOT style
gbl.gStyle.SetHistLineColor(4)
gbl.gStyle.SetHistLineWidth(2)
gbl.gStyle.SetMarkerColor(4)
gbl.gStyle.SetMarkerStyle(8)
gbl.gStyle.SetPalette(1)

# Make your algorithm
class PrintCalibDataAlg(DybPythonAlg):
    "Print Calib Data Example Algorithm"
    def __init__(self,name):
        DybPythonAlg.__init__(self,name)
        return

    def initialize(self):
        status = DybPythonAlg.initialize(self)
        if status.isFailure(): return status
        self.info("initializing")

        # Example histogram: Total raw ADC sum for each trigger
        self.stats["/file1/myhists/chargeSum"] = TH1F("chargeSum",
                                   "Sum of PMT charge for each trigger",
                                   2000,0,2000)
        return SUCCESS

    def execute(self):
        self.info("executing")

        evt = self.evtSvc()

        # Access the Calib Readout Header.
        # This is a container for calibrated data
        calibHdr = evt["/Event/CalibReadout/CalibReadoutHeader"]
        if calibHdr == None:
            self.error("Failed to get current calib readout header")
            return FAILURE

        # Access the Readout.  This is the calibrated data from one trigger.
        calibReadout = calibHdr.calibReadout()
        if calibReadout == None:
            self.error("Failed to get calibrated readout from header")
            return FAILURE

        # Get the detector ID for this trigger
        detector = calibReadout.detector()
        self.info("Detector Name: "+detector.detName())

        # Trigger Type: This is an integer of the type for this trigger
        self.info("Trigger Type: "+str( calibReadout.triggerType() ))
        # Trigger Number: A count of the trigger, according to the DAQ
        self.info("Trigger Number: "+str( calibReadout.triggerNumber() ))

        # Trigger Time: Absolute time of trigger for this calibrated data
        triggerTime = calibReadout.triggerTime()
        # Trigger Time [Seconds]: Trigger time in seconds
        self.info("Trigger Time [Seconds part]: "
                  +str( triggerTime.GetSec() ))
        # Trigger Time [Nanoseconds]: Nanoseconds part of trigger time
        self.info("Trigger Time [Nanoseconds part]: "
                  +str( triggerTime.GetNanoSec() ))
        # Full Trigger Time: Seconds + nanoseconds
        # Warning: When you add this together, it will lose some precision.
        self.info("Full Trigger Time: "
                  +str( triggerTime.GetSec()
                        +triggerTime.GetNanoSec()*1.0e-9 ))
        
        # Loop over each channel data in this trigger
        chargeSum = 0
        for channelPair in calibReadout.channelReadout():
            channel = channelPair.second
            sensorId = channel.pmtSensorId()
            pmtId = AdPmtSensor( sensorId.fullPackedData() )

            # The pmt ID contains the detector ID, AD column and row numbers.
            self.info("Pmt ID:"
                      +" detector="
                      +pmtId.detName()
                      +" column="
                      +str(pmtId.column())
                      +" ring="
                      +str(pmtId.ring()))

            # Calibrated hit data for this channel
            for hitIdx in range( channel.size() ):
                # Hit time is in units of ns, and is relative to trigger time
                hitTime = channel.time( hitIdx )
                self.info("Hit Time: "+str( hitTime ))
                # Hit charge is in units of photoelectrons
                hitCharge = channel.charge( hitIdx )
                self.info("Hit Charge: "+str( hitCharge ))
                chargeSum += hitCharge

        # Add this trigger to histogram of Charge sum
        self.stats["/file1/myhists/chargeSum"].Fill( chargeSum )
        return SUCCESS
        
    def finalize(self):
        self.info("finalizing")
        status = DybPythonAlg.finalize(self)
        return status

#####  Job Configuration for nuwa.py ########################################

def configure( argv=[] ):
    """ Example of processing calibrated data """

    # Setup root file for output histograms
    from StatisticsSvc.StatisticsSvcConf import StatisticsSvc
    statsSvc = StatisticsSvc()
    statsSvc.Output = {"file1":"calibDataResult.root"}

    return

def run(app):
    '''
    Configure and add the algorithm to job
    '''
    app.ExtSvc += ["StatisticsSvc"]
    myAlg = PrintCalibDataAlg("MyPrintCalibDataAlg")
    app.addAlgorithm(myAlg)
    pass

