#!/usr/bin/env python
#
# Example script for printing raw data to the terminal, and makes one histogram
#
# This script contains comments describing the contents of the raw data. 
#
#  Usage:
#   nuwa.py -A None -n -1 -m"Quickstart.PrintRawData" daq.NoTag.....data
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
class PrintRawDataAlg(DybPythonAlg):
    "Print Raw Data Example Algorithm"
    def __init__(self,name):
        DybPythonAlg.__init__(self,name)
        return

    def initialize(self):
        status = DybPythonAlg.initialize(self)
        if status.isFailure(): return status
        self.info("initializing")

        # Example histogram: Total raw ADC sum for each trigger
        self.stats["/file1/myhists/adcSum"] = TH1F("adcSum",
                                   "Sum of raw ADC values for each trigger",
                                   2000,0,20000)
        return SUCCESS

    def execute(self):
        self.info("executing")

        evt = self.evtSvc()

        # Access the Readout Header.  This is a container for the readout data
        readoutHdr = evt["/Event/Readout/ReadoutHeader"]
        if readoutHdr == None:
            self.error("Failed to get current readout header")
            return FAILURE

        # Access the Readout.  This is the data from one trigger.
        readout = readoutHdr.readout()
        if readout == None:
            daqReadout = readoutHdr.daqReadout()
            if daqReadout == None:
                self.error("Failed to get readout from header")
                return FAILURE
            daqXmlStream = gbl.DybDaq.DaqXmlStream(gbl.cout,
                                                   "  ",
                                                   True)
            daqXmlStream.dumpElement(daqReadout.eventReadout())
            return SUCCESS

        # Get the detector ID for this trigger
        detector = readout.detector()
        self.info("Detector Name: "+detector.detName())

        # Trigger Type: This is an integer of the type for this trigger
        self.info("Trigger Type: "+str( readout.triggerType() ))
        # Trigger Number: A count of the trigger, according to the DAQ
        self.info("Trigger Number: "+str( readout.triggerNumber() ))

        # Trigger Time: Absolute time of trigger for this raw data
        triggerTime = readout.triggerTime()
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
        
        # Loop over each channel data in this trigger
        adcSum = 0
        for channelPair in readout.channelReadout():
            channel = channelPair.second
            channelId = channel.channelId()

            # The channel ID contains the detector ID, electronics board number,
            # and the connector number on the board.
            self.info("Channel ID:"
                      +" detector="
                      +channelId.detName()
                      +" board="
                      +str(channelId.board())
                      +" connector="
                      +str(channelId.connector()))

            # TDC data for this channel
            #
            # The TDC is an integer count of the time between the time
            # the PMT pulse arrived at the channel, and the time the
            # trigger reads out the data.  Therefore, a larger TDC =
            # earlier time.  One TDC count ~= 1.5625 nanoseconds.
            #
            for tdcIdx in range( channel.size() ):
                tdc = channel.tdc( tdcIdx )
                self.info("TDC value: "+str( tdc ))

            # ADC data for this channel
            #
            # The ADC is an integer count of the charge of the PMT
            # pulse.  It is 12 bits (0 to 4095).  There are two ADCs
            # for every PMT channel (High gain and Low gain).  Only
            # the high gain ADC is recorded by default.  If the high
            # gain ADC is saturated (near 4095), then the low gain ADC
            # is recorded instead.
            #
            # For the Mini Dry Run data, one PMT photoelectron makes
            # about 20 high gain ADC counts and about 1 low gain ADC
            # count.  There is an offset (Pedestal) for each ADC of
            # ~70 ADC counts (ie. no signal = ~70 ADC, 1 photoelectron
            # = ~90 ADC, 2 p.e. = ~110 ADC, etc.)
            #
            # The ADC clock is a record of the clock cycle which had
            # the 'peak' ADC.
            #
            # ADC Gain: Here is a description of ADC gain for these ADC values
            #  Unknown = 0
            #  High = 1
            #  Low = 2
            #
            # Loop over each ADC value
            for adcIdx in range( channel.size() ):
                adc = channel.adc( adcIdx )
                pedestal = 0
                if channel.pedestal().size()>0:
                    pedestal = channel.pedestal( adcIdx )
                adcClock = channel.adcCycle( adcIdx )
                adcGain = channel.adcRange( adcIdx )
                self.info("ADC value: "+str(adc)
                          + " (pedestal: "+str( pedestal )+","
                          + " peak cycle: "+str( adcClock )+","
                          + " gain: "+str( adcGain )+")")
                # Add to total ADC sum for this trigger
                if adcGain == 1:
                    adcSum += (adc-pedestal)
                elif adcGain == 2:
                    # Adjust low gain adc to high gain scale
                    adcSum += (adc-pedestal) * 20

        # Add this trigger to histogram of ADC sum
        self.stats["/file1/myhists/adcSum"].Fill( adcSum )
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
    myAlg = PrintRawDataAlg("MyPrintRawDataAlg")
    app.addAlgorithm(myAlg)
    pass

