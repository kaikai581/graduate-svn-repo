#!/usr/bin/env python

'''
usage example:

  nuwa.py -A None -n -1 -m"Quickstart.Calibrate" -m"Quickstart.Reconstruct" daq.NoTag....data

'''

import os

def configure(argv = []):
    """ Example of reconstructing calibrated data """
    # Configure Pmt Geometry service
    from DetHelpers.DetHelpersConf import PmtGeomInfoSvc
    pgisvc = PmtGeomInfoSvc("PmtGeomInfoSvc")
    pgisvc.StreamItems = [ "/dd/Structure/DayaBay" ]

    from QsumEnergy.QsumEnergyConf import QsumEnergyTool
    qsumEnergyTool = QsumEnergyTool("AdQsumEnergyTool")
    from CenterOfChargePos.CenterOfChargePosConf import CenterOfChargePosTool
    cocPosTool = CenterOfChargePosTool("AdCenterOfChargePosTool")
    
    import ReconAlg
    recAlg = ReconAlg.Configure()
    recAlg.setReconStyle( { "AdSimple"  : [qsumEnergyTool.getFullName(),
                                           cocPosTool.getFullName()]
                            } )
    
def run(app):
    pass
