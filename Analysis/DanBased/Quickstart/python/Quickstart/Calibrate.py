#!/usr/bin/env python
#
# Example module for calibration of a raw data file
#
#  Usage:
#   nuwa.py -A None -n -1 -m"Calibrate" daq...data
#
#

def configure( argv=[] ):
    """ Example of calibrating raw data """

    # Add Calibration Algorithm
    from Gaudi.Configuration import ApplicationMgr
    app = ApplicationMgr()
    app.TopAlg.append("CalibAlg")
    return

def run(app):
    '''
    Run Time configuration
    '''
    pass
