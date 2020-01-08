def configure(argv = []):
  # parse command line arguments of this module
  from optparse import OptionParser
  parser = OptionParser()
  
  parser.add_option("-o", "--output", action="store", type="string",
                    default = "rawTree.root", help="Root tree file name")
                    
  (opts, args) = parser.parse_args(argv)
  
  from Gaudi.Configuration import ApplicationMgr
  theApp = ApplicationMgr()

  from RpcRawTree.RpcRawTreeConf import RpcRawTreeAlg
  myAlg = RpcRawTreeAlg()
  theApp.TopAlg.append(myAlg)
  myAlg.fileName = opts.output
#  self.info('output to file: ' + myAlg.fileName)
