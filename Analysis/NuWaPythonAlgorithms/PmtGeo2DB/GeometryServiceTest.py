#!/usr/bin/env python


#   Usage:
#   $ nuwa.py -n 1 -m"GeometryServiceTest" /eliza16/dayabay/data/exp/dayabay/2012/p12b/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root



# Load DybPython
from DybPython.DybPythonAlg import DybPythonAlg
from GaudiPython import SUCCESS, FAILURE
from GaudiPython import gbl

XYZPoint = gbl.ROOT.Math.XYZPoint

# Algorithm for testing NuWa geometry service
class GeometryServiceTest(DybPythonAlg):
  "An algorithm for testing NuWa geometry service"
  def __init__(self, name):
    DybPythonAlg.__init__(self, name)
    return
  
  def initialize(self):
    status = DybPythonAlg.initialize(self)
    if status.isFailure(): return status
    self.info("initializing")
    
    self.coordSysSvc = self.svc('ICoordSysSvc','CoordSysSvc')
    if self.coordSysSvc == None:
      self.error("Failed to get CoordSysSvc")
      return FAILURE
    site1 = self.getDet("/dd/Structure/Sites/db-rock")
    if not site1:
      self.error("Failed to get site1 DE")
      return FAILURE
    eh1ad1 = self.getDet("/dd/Structure/AD/db-iav1")
    #eh1ad1 = self.getDet("/dd/Structure/AD/db-ade1")
    #eh1ad1 = self.getDet("/dd/Structure/AD/db-sst1")
    if not site1:
      self.error("Failed to get EH1 AD1 DE")
      return FAILURE
    eh1ad2 = self.getDet("/dd/Structure/AD/db-ade2")
    if not site1:
      self.error("Failed to get EH1 AD2 DE")
      return FAILURE
    eh1iws = self.getDet("/dd/Structure/Pool/db-iws")
    if not site1:
      self.error("Failed to get EH1 IWS DE")
      return FAILURE
    eh1ows = self.getDet("/dd/Structure/Pool/db-ows")
    if not site1:
      self.error("Failed to get EH1 OWS DE")
      return FAILURE
    eh1pool = self.getDet("/dd/Structure/Pool")
    if not site1:
      self.error("Failed to get EH1 pool DE")
      return FAILURE

    origin = XYZPoint(0,0,0)
    gSite1O = site1.geometry().toGlobal(origin)
    self.info("EH1 site origin in global coordinate system (x,y,z):")
    self.info("("+str(gSite1O.x())+","+str(gSite1O.y())+","+
              str(gSite1O.z())+")")
    
    geh1ad1 = eh1ad1.geometry().toGlobal(origin)
    geh1ad2 = eh1ad2.geometry().toGlobal(origin)
    geh1iws = eh1iws.geometry().toGlobal(origin)
    geh1ows = eh1ows.geometry().toGlobal(origin)
    #eh1ad1iws = XYZPoint(geh1ad1.x()-geh1iws.x(),geh1ad1.y()-geh1iws.y(),geh1ad1.z()-geh1iws.z())
    eh1ad1site = XYZPoint(geh1ad1.x()-gSite1O.x(),geh1ad1.y()-gSite1O.y(),geh1ad1.z()-gSite1O.z())
    eh1iwsows = XYZPoint(geh1iws.x()-geh1ows.x(),geh1iws.y()-geh1ows.y(),geh1iws.z()-geh1ows.z())
    self.info("EH1 AD1 origin in pool coordinate system (x,y,z):")
    #self.info("("+str(eh1ad1iws.x())+","+str(eh1ad1iws.y())+","+str(eh1ad1iws.z())+")")
    self.info("("+str(eh1ad1site.x())+","+str(eh1ad1site.y())+","+str(eh1ad1site.z())+")")
    self.info("("+str(eh1iwsows.x())+","+str(eh1iwsows.y())+","+str(eh1iwsows.z())+")")
    

    return SUCCESS
  
  def execute(self):
    """event loop"""
    evt = self.evtSvc()
    self.info("executing GeometryServiceTest")
    
    deEh1Ad1 = self.getDet("/dd/Structure/AD/db-ade1")
    if not deEh1Ad1:
      self.error("Failed to get DE")
      return FAILURE
    
    origin = XYZPoint(0,0,0)
    gloEh1Ad1Origin = deEh1Ad1.geometry().toGlobal(origin)
    self.info("EH1 AD1 origin in global coordinate system (x,y,z):")
    self.info("("+str(gloEh1Ad1Origin.x())+","+str(gloEh1Ad1Origin.y())+","+
              str(gloEh1Ad1Origin.z())+")")
    
    deEh2Ad1 = self.getDet("/dd/Structure/AD/la-ade1")
    if not deEh2Ad1:
      self.error("Failed to get DE")
      return FAILURE
    
    gloEh2Ad1Origin = deEh2Ad1.geometry().toGlobal(origin)
    self.info("EH2 AD1 origin in global coordinate system (x,y,z):")
    self.info("("+str(gloEh2Ad1Origin.x())+","+str(gloEh2Ad1Origin.y())+","+
              str(gloEh2Ad1Origin.z())+")")

    deDB = self.getDet("/dd/Structure/Pool/db-iws")
    if not deDB:
      self.error("Failed to get EH1 IWS DE")
      return FAILURE
    deLA = self.getDet("/dd/Structure/Pool/la-iws")
    if not deLA:
      self.error("Failed to get EH2 IWS DE")
      return FAILURE
    
    gEh1Iws = deDB.geometry().toGlobal(origin)
    gEh2Iws = deLA.geometry().toGlobal(origin)
    self.info("EH1 IWS origin in global coordinate system (x,y,z):")
    self.info("("+str(gEh1Iws.x())+","+str(gEh1Iws.y())+","+
              str(gEh1Iws.z())+")")
    self.info("EH2 IWS origin in global coordinate system (x,y,z):")
    self.info("("+str(gEh2Iws.x())+","+str(gEh2Iws.y())+","+
              str(gEh2Iws.z())+")")
    
    return SUCCESS


  def finalize(self):
    self.info("finalizing")
    status = DybPythonAlg.finalize(self)
    return status



#####  Job Configuration for nuwa.py ###########################################

def configure(argv=[]):
  # Process module options
  return



def run(app):
  app.ExtSvc += ["StatisticsSvc","CoordSysSvc"]
  myAlg = GeometryServiceTest("GeometryServiceTest")
  app.addAlgorithm(myAlg)
  pass
