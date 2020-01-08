#!/usr/bin/env python
#
#
#  Usage:
#   nuwa.py -A None -n 1 --dbconf=offline_db_pdsf -m"DayaBayGeo2DB_v04"
#
#

# This script is to write the PMT and RPC strip geometry information into an 
# existing MySQL database. Information includes global coordinates of
# the center of water pool, AD PMTs and RPC strips.


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
class DayaBayGeo2DB_v04(DybPythonAlg):
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
    
    # RPC Geometry Service: This service gives you information
    # about the RPC strip positions in the modules
    self.rpcGeomSvc = self.svc('IRpcGeomInfoSvc','RpcGeomInfoSvc')
    if self.rpcGeomSvc == None:
      self.error("Failed to get RpcGeomInfoSvc")
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
    
    sitelist  = [Site.kDayaBay, Site.kLingAo, Site.kFar]
    addetlist = [DetectorId.kAD1, DetectorId.kAD2, DetectorId.kAD3, DetectorId.kAD4]
    wpdetlist = [DetectorId.kIWS, DetectorId.kOWS]
    rpcdet    = DetectorId.kRPC
    # connect to heplinux9
    db = MySQLdb.connect(read_default_file="~/.my.cnf", read_default_group="hep9")
    for site in sitelist:
      siteOrigin = eh[site].geometry().toGlobal(XYZPoint(0,0,0))
      site_cursor = db.cursor()
      site_cursor.execute("""INSERT IGNORE INTO hall (hallId, coordOriginX, coordOriginY, coordOriginZ) VALUES (%s, %s, %s, %s)""", (site, siteOrigin.x(),siteOrigin.y(),siteOrigin.z()))
      db.commit()
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
        if adPmtSensors.size() != 0:
          # write detector information to detector table
          det_cursor = db.cursor()
          det_cursor.execute("""INSERT IGNORE INTO detector (hallId, detectorId) VALUES (%s, %s)""", (site, addet))
          db.commit()
        self.info("number of PMTs "+str(adPmtSensors.size()))
        
        for sens in adPmtSensors:
          # Get the position of this PMT:
          pmtGeomInfo = self.pmtGeomSvc.get(sens.fullPackedData())
          pmtXYZ = pmtGeomInfo.globalPosition()
          pmtNormal = pmtGeomInfo.globalDirection()
          csr = db.cursor()
          csr.execute("""INSERT IGNORE INTO adPmt (hallId, detectorId, ring, col, centerX, centerY, centerZ, normalX, normalY, normalZ) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s)""", (site, addet, sens.ring(),sens.column(),pmtXYZ.x(),pmtXYZ.y(),pmtXYZ.z(),pmtNormal.x(),pmtNormal.y(),pmtNormal.z()))
          db.commit()
        
      # write water pool PMT informaition
      for wpdet in wpdetlist:
        context.SetDetId(wpdet)
        self.info(context.AsString())
        svcMode = ServiceMode(context, 0)
        poolPmtSensors = self.cableSvc.poolPmtSensors(svcMode)
        if poolPmtSensors.size() != 0:
          # write detector information to detector table
          det_cursor = db.cursor()
          det_cursor.execute("""INSERT IGNORE INTO detector (hallId, detectorId) VALUES (%s, %s)""", (site, wpdet))
          db.commit()
        self.info("number of PMTs "+str(poolPmtSensors.size()))
        
        for sens in poolPmtSensors:
          # Get the position of this PMT:
          pmtGeomInfo = self.pmtGeomSvc.get(sens.fullPackedData())
          pmtXYZ = pmtGeomInfo.globalPosition()
          pmtNormal = pmtGeomInfo.globalDirection()
          self.debug(str(sens.wallNumber())+" "+str(sens.wallSpot())+" "
                    +str(sens.inwardFacing())+"\t"
                    +str(pmtXYZ.x())+" "+str(pmtXYZ.y())+" "
                    +str(pmtXYZ.z()))
          csr = db.cursor()
          csr.execute("""INSERT IGNORE INTO wsPmt (hallId, detectorId, wallNumber, wallSpot, inwardFacing, centerX, centerY, centerZ, normalX, normalY, normalZ) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)""", (site, wpdet, sens.wallNumber(),sens.wallSpot(),sens.inwardFacing(),pmtXYZ.x(),pmtXYZ.y(),pmtXYZ.z(),pmtNormal.x(),pmtNormal.y(),pmtNormal.z()))
          db.commit()
      
      # write RPC strip informaition
      context.SetDetId(rpcdet)
      self.info(context.AsString())
      svcMode = ServiceMode(context, 0)
      # Be careful! If --dbconf=offline_db_pdsf is not secified, no RPC
      # sensors can be obtained!
      rpcSensors = self.cableSvc.rpcSensors(svcMode)
      if rpcSensors.size() != 0:
        # write detector information to detector table
        det_cursor = db.cursor()
        det_cursor.execute("""INSERT IGNORE INTO detector (hallId, detectorId) VALUES (%s, %s)""", (site, rpcdet))
        db.commit()
      self.info("number of strips "+str(rpcSensors.size()))
      
      for sens in rpcSensors:
        # Get the position of this strip:
        rpcGeomInfo = self.rpcGeomSvc.get(sens.fullPackedData())
        stripXYZ = rpcGeomInfo.globalPosition()
        stripNormal = rpcGeomInfo.globalDirection()
        csr = db.cursor()
        csr.execute("""INSERT IGNORE INTO rpcStrip (hallId, detectorId, row, col, layer, strip, centerX, centerY, centerZ, normalX, normalY, normalZ) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)""", (site, rpcdet, sens.panelRow(),sens.panelColumn(),sens.layer(),sens.strip(),stripXYZ.x(),stripXYZ.y(),stripXYZ.z(),stripNormal.x(),stripNormal.y(),stripNormal.z()))
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
  # configure RpcGeomInfoSvc
  from DetHelpers.DetHelpersConf import RpcGeomInfoSvc
  rpgisvc = RpcGeomInfoSvc("RpcGeomInfoSvc")
  rpgisvc.StreamItems = [ "/dd/Structure/DayaBay" ]
  return

def run(app):
  '''
  Configure and add the algorithm to job
  '''
  #app.ExtSvc += ["StaticCableSvc"]
  app.ExtSvc += ["PmtGeomInfoSvc"]
  app.ExtSvc += ["RpcGeomInfoSvc"]
  myAlg = DayaBayGeo2DB_v04("GeometryToDatabaseAlg")
  app.addAlgorithm(myAlg)
  pass
