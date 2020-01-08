#!/usr/bin/env python
#
# Load Cable map and Calibration tables for Dry Run data
#
#  Usage:
#   nuwa.py -A None -n -1 -m"DryRunTables" -m"Calibrate" daq...data
#
#

def configure( argv=[] ):
    """ Example of loading Dry Run cabling and calibration tables"""

    # Add Static Tables
    import DataSvc
    DataSvc.Configure(
        pmtCalibDataPath="$DATASVCROOT/share/DryRun/pmtCalibTable_DryRunNominal.txt",
        feeCableMapPath="$DATASVCROOT/share/DryRun/feeCableMap_DryRun.txt")
    return

def run(app):
    '''
    Run Time configuration
    '''
    pass
