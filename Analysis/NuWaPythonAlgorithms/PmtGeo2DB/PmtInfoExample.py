#!/usr/bin/env python
#
# Example script for reading raw data and using services to access the PMT ID
# and position.
#
#  Usage:
#   nuwa.py -A None -n -1 -m"Quickstart.PmtInfoExample" daq.NoTag.....data
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
TH2F = gbl.TH2F
gPad = gbl.gPad
TimeStamp = gbl.TimeStamp
FeeChannelId = gbl.DayaBay.FeeChannelId
AdPmtSensor = gbl.DayaBay.AdPmtSensor
Detector = gbl.DayaBay.Detector
ServiceMode = gbl.ServiceMode
Site = gbl.Site

#change default ROOT style
gbl.gStyle.SetHistLineColor(4)
gbl.gStyle.SetHistLineWidth(2)
gbl.gStyle.SetMarkerColor(4)
gbl.gStyle.SetMarkerStyle(8)
gbl.gStyle.SetPalette(1)

import math

# Make your algorithm
class PmtInfoExampleAlg(DybPythonAlg):
    "Process raw data and access the related PMT information"
    def __init__(self,name):
        DybPythonAlg.__init__(self,name)
        return

    def initialize(self):
        status = DybPythonAlg.initialize(self)
        if status.isFailure(): return status
        self.info("initializing")

        # Prepare the services here:

        # Cable Service: This service provides a mapping between
        # electronics channels and pmts in the detector.
        self.cableSvc = self.svc('ICableSvc','CableSvc')
        if self.cableSvc == None:
            self.error("Failed to get CableSvc")
            return FAILURE

        # PMT Geometry Service: This service gives you information
        # about the PMT positions in the detectors
        self.pmtGeomSvc = self.svc('IPmtGeomInfoSvc','PmtGeomInfoSvc')
        if self.pmtGeomSvc == None:
            self.error("Failed to get PmtGeomInfoSvc")
            return FAILURE

        # Example histogram: Count of hits on PMTs in AD
        self.stats["/file1/myhists/pmtHits"] = TH2F("pmtHits",
                                                    "Count of PMT hits",
                                                    49,-3.25,360+3.25,
                                                    19,-2.5+0.125,2.5-0.125)
        self.stats["/file1/myhists/pmtHits"].GetXaxis().SetLabelSize(0)
        self.stats["/file1/myhists/pmtHits"].GetXaxis().SetTitle("PMT columns")
        self.stats["/file1/myhists/pmtHits"].GetYaxis().SetTitle("PMT rings [m]")

        return SUCCESS

    def execute(self):
        """Process one triggered AD Readout"""
        evt = self.evtSvc()

        # Access the Readout Header.  This is a container for the readout data
        readoutHdr = evt["/Event/Readout/ReadoutHeader"]
        if readoutHdr == None:
            self.error("Failed to get current readout header")
            return FAILURE

        # Access the Readout.  This is the data from one trigger.
        readout = readoutHdr.daqCrate().asPmtCrate()
        if readout == None:
            self.info("No PMT readout this cycle")
            return SUCCESS

        # The Service mode is used to get the correct cable mapping, etc.
        # You set it using the context (time, location) of the current data
        svcMode = ServiceMode( readoutHdr.context(), 0 )

        # Loop over each channel data in this trigger
        for channel in readout.channelReadouts():
            channelId = channel.channelId()

            # Get the PMT ID attached to this electronics channel
            pmtId = None
            if channelId.isAD():
                pmtId = self.cableSvc.adPmtSensor(channelId, svcMode)
            elif channelId.isWaterShield():
                pmtId = self.cableSvc.poolPmtSensor(channelId, svcMode)
                
            # Skip channels not connected to a PMT
            if pmtId.sensorId()==0: continue

            # Skip water pool pmts in this example script
            if pmtId.isWaterShield(): continue

            # The AD PMT ID gives you information about the PMT:
            # pmtId.ring()        Ring in AD
            # pmtId.column()      Column in AD
            # pmtId.detectorId()  Detector ID 
            # pmtId.site()        Site ID
            # pmtId.detName()     Combined detector name: "SiteDetector"

            # Get the position of this PMT:
            pmtGeomInfo = self.pmtGeomSvc.get( pmtId.fullPackedData() )
            pmtXYZ = pmtGeomInfo.localPosition()

            # The PMT position is a vector of the PMT 
            # pmtXYZ.x()          X position in AD or water pool
            # pmtXYZ.y()          Y position in AD or water pool
            # pmtXYZ.z()          Z position in AD or water pool

            # Make a new coordinate 'pmtPhiPrime'.  Based on PMT Phi,
            # but in correct orientation
            pmtPhi = pmtXYZ.phi() * 180/math.pi
            pmtZ = pmtXYZ.z()
            if pmtId.ring()==0:
                pmtZ *= -1
            if pmtPhi<0: pmtPhi+= 360.
            pmtPhiPrime = 360-pmtPhi
            self.stats["/file1/myhists/pmtHits"].Fill(pmtPhiPrime,
                                                      pmtZ/units.m)
        return SUCCESS
        
    def finalize(self):
        self.info("finalizing")
        # Draw Figures:
        pmtHits = self.stats["/file1/myhists/pmtHits"].Draw("colz")
        gPad.SaveAs("pmtHits.png")

        status = DybPythonAlg.finalize(self)
        
        return status

#####  Job Configuration for nuwa.py ########################################

def configure( argv=[] ):
    """ Example of processing raw data """

    # Setup root file for output histograms
    from StatisticsSvc.StatisticsSvcConf import StatisticsSvc
    statsSvc = StatisticsSvc()
    statsSvc.Output = {"file1":"pmtInfoResult.root"}

    # Setup Service to get PMT position in detectors
    from DetHelpers.DetHelpersConf import PmtGeomInfoSvc
    pgiSvc = PmtGeomInfoSvc()
    pgiSvc.StreamItems = ["/dd/Structure/DayaBay"]

    return

def run(app):
    '''
    Configure and add the algorithm to job
    '''
    app.ExtSvc += ["StatisticsSvc","PmtGeomInfoSvc"]
    myAlg = PmtInfoExampleAlg("MyPmtInfoExampleAlg")
    app.addAlgorithm(myAlg)
    pass

