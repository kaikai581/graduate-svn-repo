#!/usr/bin/env python
#
# Example script for printing reconstructed data to the terminal, and makes one histogram
#
# This script contains comments describing the contents of the raw data. 
#
#  Usage:
#   nuwa.py -A None -n -1 -m"Quickstart.Calibrate" -m"Quickstart.Reconstruct" -m"Quickstart.PrintReconData" daq.NoTag.....data
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
FeeChannelId = gbl.DayaBay.FeeChannelId
Detector = gbl.DayaBay.Detector

#change default ROOT style
gbl.gStyle.SetHistLineColor(4)
gbl.gStyle.SetHistLineWidth(2)
gbl.gStyle.SetMarkerColor(4)
gbl.gStyle.SetMarkerStyle(8)
gbl.gStyle.SetPalette(1)

# Make your algorithm
class PrintReconDataAlg(DybPythonAlg):
    "Print Reconstructed Data Example Algorithm"
    def __init__(self,name):
        DybPythonAlg.__init__(self,name)
        return

    def initialize(self):
        status = DybPythonAlg.initialize(self)
        if status.isFailure(): return status
        self.info("initializing")

        # Example histogram: Total raw ADC sum for each trigger
        self.stats["/file1/myhists/energy"] = TH1F("energy",
                                   "Reconstructed energy for each trigger",
                                   2000,0,20000)
        return SUCCESS

    def execute(self):
        self.info("executing")

        evt = self.evtSvc()

        # Access the Recon Header.  This is a container for the reconstructed data
        reconHdr = evt["/Event/Rec/RecHeader"]
        if reconHdr == None:
            self.error("Failed to get current recon header")
            return FAILURE

        # Access the list of reconstruction results.  Multiple reconstruction types can be run in parallel
        for resultPair in reconHdr.recResults():
            result = resultPair.second
            self.info("Reconstruction Name: "+result.name())            

            # Get the detector ID for this trigger
            detector = result.detector()
            self.info("Detector Name: "+detector.detName())

            # Trigger Type: This is an integer of the type for this trigger
            self.info("Trigger Type: "+str( result.triggerType() ))
            # Trigger Number: A count of the trigger, according to the DAQ
            self.info("Trigger Number: "+str( result.triggerNumber() ))

            # Trigger Time: Absolute time of trigger for this raw data
            triggerTime = result.triggerTime()
            # Trigger Time [Seconds]: Trigger time in seconds from some day in 1990
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

            # Energy information
            self.info("Energy Status: "+str( result.energyStatus() ))
            self.info("Energy: "+str( result.energy()/units.MeV )+" MeV")
            self.info("Energy Quality: "+str( result.energyQuality() ))
            # Add this trigger to histogram of energy
            self.stats["/file1/myhists/energy"].Fill( result.energy()/units.MeV )
            
            # Position information
            self.info("Position Status: "+str( result.positionStatus() ))
            self.info("Position (X,Y,Z): %f, %f, %f [mm]" % (result.position().x()/units.mm,
                                                             result.position().y()/units.mm,
                                                             result.position().z()/units.mm) )
            self.info("Position Quality: "+str( result.positionQuality() ))

            # Direction information, for tracks
            self.info("Direction Status: "+str( result.directionStatus() ))
            self.info("Direction (X,Y,Z): %f, %f, %f" % (result.direction().x(),
                                                         result.direction().y(),
                                                         result.direction().z()) )
            self.info("Direction Quality: "+str( result.directionQuality() ))


            # Covariance Matrix, if one is generated
            result.errorMatrix()

        return SUCCESS
        
    def finalize(self):
        self.info("finalizing")
        status = DybPythonAlg.finalize(self)
        return status

#####  Job Configuration for nuwa.py ########################################

def configure( argv=[] ):
    """ Example of processing reconstructed data """

    # Setup root file for output histograms
    from StatisticsSvc.StatisticsSvcConf import StatisticsSvc
    statsSvc = StatisticsSvc()
    statsSvc.Output = {"file1":"reconDataResult.root"}
    return

def run(app):
    '''
    Configure and add the algorithm to job
    '''
    app.ExtSvc += ["StatisticsSvc"]
    myAlg = PrintReconDataAlg("MyPrintReconDataAlg")
    app.addAlgorithm(myAlg)
    pass

