#!/usr/bin/env python
#
#
#  Usage:
#   nuwa.py -A None -n 1 -m"Geom2DB" /eliza16/dayabay/data/exp/dayabay/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root
#
#

# This script is to write the PMT geometry information into an existing
# MySQL database. Information includes global and site coordinates of the center
# of water pool and AD PMTs.


# Load DybPython
from DybPython.DybPythonAlg import DybPythonAlg
# * also includes gbl
from GaudiPython import *

import os
os.environ.setdefault( 'DBCONF',  "offline_db")

# MySQL-python connection
import MySQLdb

# function shortcuts
ServiceMode = gbl.ServiceMode
Context = gbl.Context
Site = gbl.Site
DetectorId = gbl.DetectorId
SimFlag = gbl.SimFlag
TimeStamp = gbl.TimeStamp
XYZPoint = gbl.ROOT.Math.XYZPoint

# Make your algorithm
class Geom2DB(DybPythonAlg):
  "Write NuWa geometry information to database"
  def __init__(self,name):
    DybPythonAlg.__init__(self,name)
    return

  def initialize(self):
    status = DybPythonAlg.initialize(self)
    if status.isFailure(): return status
    
    # PMT Geometry Service: This service gives you information
    # about the PMT positions in the detectors
    self.pmtGeomSvc = self.svc('IPmtGeomInfoSvc','PmtGeomInfoSvc')
    if self.pmtGeomSvc == None:
      self.error("Failed to get PmtGeomInfoSvc")
      return FAILURE
    
    eh = {}
    eh[Site.kDayaBay] = self.getDet("/dd/Structure/Sites/db-rock")
    eh[Site.kLingAo] = self.getDet("/dd/Structure/Sites/la-rock")
    eh[Site.kFar] = self.getDet("/dd/Structure/Sites/far-rock")
    if not eh[Site.kDayaBay]:
      self.error("Failed to get EH1 DE")
      return FAILURE
    if not eh[Site.kLingAo]:
      self.error("Failed to get EH2 DE")
      return FAILURE
    if not eh[Site.kFar]:
      self.error("Failed to get EH3 DE")
      return FAILURE
    
    # There are several different kinds of cable map services available.
    # Used the simplest one for this purpose.
    #self.cableSvc = self.svc('ICableSvc', 'DbiCableSvc')
    #self.cableSvc = self.svc('ICableSvc', 'StaticCableSvc')
    self.cableSvc = self.svc('ICableSvc', 'CableSvc')
    self.info("initializing")
    
    sitelist = [Site.kDayaBay, Site.kLingAo, Site.kFar]
    addetlist = [DetectorId.kAD1, DetectorId.kAD2, DetectorId.kAD3, DetectorId.kAD4]
    wpdetlist = [DetectorId.kIWS, DetectorId.kOWS]
    for site in sitelist:
      siteOrigin = eh[site].geometry().toGlobal(XYZPoint(0,0,0))
      # connect to heplinux3
      db = MySQLdb.connect(read_default_file="~/.my.cnf", read_default_group="hep3")
      context = Context()
      context.SetSite(site)
      context.SetSimFlag(SimFlag.kData)
      context.SetTimeStamp(TimeStamp(2012,1,1,0,0,0))
      
      # write AD PMT informaition
      for addet in addetlist:
        context.SetDetId(addet)
        self.info(context.AsString())
        svcMode = ServiceMode(context, 0)
        adPmtSensors = self.cableSvc.adPmtSensors(svcMode)
        self.info("number of PMTs "+str(adPmtSensors.size()))
        
        for sens in adPmtSensors:
          # Get the position of this PMT:
          pmtGeomInfo = self.pmtGeomSvc.get(sens.fullPackedData())
          pmtXYZ = pmtGeomInfo.globalPosition()
          csr = db.cursor()
          csr.execute("""INSERT IGNORE INTO ad_pmt_locations (site, detector, ring, `column`, x_site, y_site, z_site, x_global, y_global, z_global) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s)""", (site, addet, sens.ring(),sens.column(),pmtXYZ.x()-siteOrigin.x(),pmtXYZ.y()-siteOrigin.y(),pmtXYZ.z()-siteOrigin.z(),pmtXYZ.x(),pmtXYZ.y(),pmtXYZ.z()))
          db.commit()
        
      # write water pool PMT informaition
      for wpdet in wpdetlist:
        context.SetDetId(wpdet)
        self.info(context.AsString())
        svcMode = ServiceMode(context, 0)
        poolPmtSensors = self.cableSvc.poolPmtSensors(svcMode)
        self.info("number of PMTs "+str(poolPmtSensors.size()))
        
        for sens in poolPmtSensors:
          # Get the position of this PMT:
          pmtGeomInfo = self.pmtGeomSvc.get(sens.fullPackedData())
          pmtXYZ = pmtGeomInfo.globalPosition()
          self.debug(str(sens.wallNumber())+" "+str(sens.wallSpot())+" "
                    +str(sens.inwardFacing())+"\t"
                    +str(pmtXYZ.x())+" "+str(pmtXYZ.y())+" "
                    +str(pmtXYZ.z()))
          csr = db.cursor()
          csr.execute("""INSERT IGNORE INTO wp_pmt_locations (site, detector, wall_number, wall_spot, inward_facing, x_site, y_site, z_site, x_global, y_global, z_global) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)""", (site, wpdet, sens.wallNumber(),sens.wallSpot(),sens.inwardFacing(),pmtXYZ.x()-siteOrigin.x(),pmtXYZ.y()-siteOrigin.y(),pmtXYZ.z()-siteOrigin.z(),pmtXYZ.x(),pmtXYZ.y(),pmtXYZ.z()))
          db.commit()
    
    return SUCCESS
    
  def execute(self):
    """Skip event loop completely"""
    return SUCCESS

  def finalize(self):
    status = DybPythonAlg.finalize(self)
    return status


#####  Job Configuration for nuwa.py ###########################################

def configure( argv=[] ):
  """ Algorithm of writing NuWa geometry information to database """
  return

def run(app):
  '''
  Configure and add the algorithm to job
  '''
  #app.ExtSvc += ["DbiCableSvc"]
  #app.ExtSvc += ["StaticCableSvc"]
  app.ExtSvc += ["PmtGeomInfoSvc"]
  myAlg = Geom2DB("GeometryToDatabaseAlg")
  app.addAlgorithm(myAlg)
  pass
