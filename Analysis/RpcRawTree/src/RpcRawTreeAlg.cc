#include "RpcRawTreeAlg.h"
#include "Event/CalibReadout.h"
#include "Event/CalibReadoutRpcCrate.h"
#include "Event/CalibReadoutRpcChannel.h"
#include "TROOT.h"
#include <cmath>
#include <sstream>


RpcRawTreeAlg::RpcRawTreeAlg(const std::string& name,
	ISvcLocator* pSvcLocator):GaudiAlgorithm(name, pSvcLocator),
	m_firstRPCEvt2f(true), m_firstRPCEvt3f(true), m_firstRPCEvt4f(true),
	m_firstRPCEvt234f(true), m_firstRPCEvt34f(true)
{
  declareProperty("fileName", m_outputFileName = "rawTree.root",
                  "output root file name");
}

RpcRawTreeAlg::~RpcRawTreeAlg(){}

StatusCode RpcRawTreeAlg::initialize()
{
  StatusCode sc;
  sc = this->GaudiAlgorithm::initialize();
  if(sc.isFailure())
  {
    error() << "Base class initialization error" << endreq;
    return sc;
  }

  m_f = new TFile(m_outputFileName.c_str(), "recreate");
  gROOT->ProcessLine(".L vecdict.h+");
  initializeTree();
  m_nRo = 0;
  cout << "initilize success" << endl;
	
  return sc;
}

StatusCode RpcRawTreeAlg::execute()
{
  StatusCode sc;
  m_nRo++;
  
  
  /// Get ReadoutHeader 
  /// =================
  ReadoutHeader* readoutHdr =
                          get<ReadoutHeader>(ReadoutHeader::defaultLocation());
  if(!readoutHdr) {
    error()<<"Failed to get readout header"<<endreq;
    return StatusCode::FAILURE;
  }
  
  sc = writeReadoutHeader(readoutHdr);
  if(sc.isFailure())
  {
    error() << "Failed to write readout header" << endreq;
    return StatusCode::FAILURE;
  }
	
  return StatusCode::SUCCESS;
}

StatusCode RpcRawTreeAlg::finalize()
{
  StatusCode sc;
  sc = this->GaudiAlgorithm::finalize();
	
  m_f->Write();
  m_f->Close();
  return sc;
}


void RpcRawTreeAlg::initializeTree()
{
  m_t = new TTree("t", "RPC tree");
  m_treeStruct = new MyTree;
  m_treeStruct->forceTrigger = new vector<bool>;
  m_treeStruct->hitRow = new vector<int>;
  m_treeStruct->hitCol = new vector<int>;
  m_treeStruct->nLayers = new vector<int>;
  m_treeStruct->stripId = new vector<vector<int> >;
  m_t->Branch("roNum", &m_nRo, "roNum/I");
  m_t->Branch("site", &m_treeStruct->roSite, "site/I");
  m_t->Branch("detector", &m_treeStruct->roDetector, "detector/I");
  m_t->Branch("triggerNumber", &m_treeStruct->roTriggerNumber,
							"triggerNumber/I");
  m_t->Branch("eventNumber", &m_treeStruct->eventNumber,
              "eventNumber/i");
//  m_t->Branch("event", &m_treeStruct->event, "event/i");
  m_t->Branch("triggerSpan", &m_treeStruct->triggerSpan, "triggerSpan/D");
  
  stringstream tmpsstrm;
  tmpsstrm << "triggerType[" << MAXSTRLEN << "]/C";
  m_t->Branch("triggerType", m_treeStruct->triggerType, tmpsstrm.str().c_str());
  
  m_t->Branch("localTriggerNumber", &m_treeStruct->localTriggerNumber,
              "localTriggerNumber/i");
              
  m_t->Branch("triggerTimeSec", &m_treeStruct->roTriggerTimeSec,
							"triggerTimeSec/I");
  m_t->Branch("triggerTimeNanoSec", &m_treeStruct->roTriggerTimeNanoSec,
							"triggerTimeNanoSec/I");
  m_t->Branch("runNumber", &m_treeStruct->runNumber,
              "runNumber/i");
  m_t->Branch("hasTriggers", &m_treeStruct->hasTriggers, "hasTriggers/O");
  m_t->Branch("eventType", &m_treeStruct->eventType, "eventType/I");
  m_t->Branch("nReadoutPanels", &m_treeStruct->nReadoutPanels,
              "nReadoutPanels/i");
  m_t->Branch("forceTrigger", &m_treeStruct->forceTrigger);
  m_t->Branch("nHitPanels", &m_treeStruct->nHitPanels, "nHitPanels/i");
  m_t->Branch("firedLayerNum", m_treeStruct->firedLayerNum,
              "firedLayerNum[nReadoutPanels]/I");
  m_t->Branch("hitRow", &m_treeStruct->hitRow);
  m_t->Branch("hitCol", &m_treeStruct->hitCol);
  m_t->Branch("nLayers", &m_treeStruct->nLayers);
  m_t->Branch("stripId", &m_treeStruct->stripId);
  m_t->Branch("fromRot", m_treeStruct->fromRot, "fromRot[nReadoutPanels]/O");


  m_t->Branch("row", m_treeStruct->roRow, "row[nStrips]/I");
  m_t->Branch("column", m_treeStruct->roColumn, "column[nStrips]/I");
  m_t->Branch("layer", m_treeStruct->roLayer, "layer[nStrips]/I");
  m_t->Branch("strip", m_treeStruct->roStrip, "strip[nStrips]/I");
  m_t->Branch("nLayersCalib", m_treeStruct->nRoLayers,
              "nLayersCalib[nStrips]/I");
}

StatusCode RpcRawTreeAlg::writeReadoutHeader(const ReadoutHeader* pReadoutHdr)
{
  
  const DaqCrate* daqCrate = pReadoutHdr->daqCrate();
  if(!daqCrate) {
    error()<<"Failed to get daqCrate from header"<<endreq;
    return StatusCode::FAILURE;
  }
  
  m_treeStruct->roDetector = daqCrate->detector().detectorId();
  m_treeStruct->roSite = daqCrate->detector().site();
  m_treeStruct->eventNumber = daqCrate->eventNumber();
//  m_treeStruct->event = daqCrate->eventReadout().header().event();
  
  const DaqRpcCrate* rpcCrate = daqCrate->asRpcCrate();
  
  
  if(!rpcCrate)
    return StatusCode::SUCCESS;

  const DaqRpcCrate::RpcPanelPtrList& rpcPanelList
                                     = rpcCrate->rpcPanelReadouts();

  info() << "RPC readout found" << endreq;
  
  info() << "trigger time sec: ";
  info() << rpcCrate->triggerTime().GetSec() << endreq;
  m_treeStruct->roTriggerTimeSec = rpcCrate->triggerTime().GetSec();
  
  info() << "trigger time nano sec: ";
  info() << rpcCrate->triggerTime().GetNanoSec() << endreq;
  m_treeStruct->roTriggerTimeNanoSec = rpcCrate->triggerTime().GetNanoSec();
  
  /// store trigger span into tree
  info() << "trigger span: " << rpcCrate->triggerSpan() << endreq;
  m_treeStruct->triggerSpan = rpcCrate->triggerSpan();
  
  /// store trigger type into tree
  info() << "trigger type: ";
  info() << DayaBay::Trigger::AsString(rpcCrate->triggerType()) << endreq;
  sprintf(m_treeStruct->triggerType, "%s",
          DayaBay::Trigger::AsString(rpcCrate->triggerType()));
  
  /// store local trigger number into tree
  info() << "local trigger number: ";
  info() << rpcCrate->localTriggerNumber() << endreq;
  m_treeStruct->localTriggerNumber = rpcCrate->localTriggerNumber();
  
  /// store run number into tree
  info() << "run number: " << rpcCrate->runNumber() << endreq;
  m_treeStruct->runNumber = rpcCrate->runNumber();
  
  /// store if there is trigger into tree
  info() << "has triggers? " << rpcCrate->hasTriggers() << endreq;
  m_treeStruct->hasTriggers = rpcCrate->hasTriggers();
  
  /// store event type into tree
  /// Rpc Trigger type, 1:only array, 2:only first telescope, 
  /// 4:only second telescope. or the mix of them.
  info() << "RPC trigger type: " << rpcCrate->getEvtType() << endreq;
  m_treeStruct->eventType = rpcCrate->getEvtType();
  
  
  unsigned int np = 0;
  m_treeStruct->forceTrigger->clear();
  m_treeStruct->hitRow->clear();
  m_treeStruct->hitCol->clear();
  m_treeStruct->nLayers->clear();
  m_treeStruct->stripId->clear();
  
  /// The following piece of code terminates nuwa. Maybe something wrong with
  /// electronics?
/*  if(!rpcPanelList.size())
  {
    info() << "no rpc strip fired in this readout" << endreq;
    return StatusCode::SUCCESS;
  }*/
  /// store number of readout modules into tree
  /// sould be compared with leaf "nModules"
  info() << "readout modules: " << rpcPanelList.size() << endreq;
  m_treeStruct->nReadoutPanels = rpcPanelList.size();
  
  
  /// start loop over readout modules
  for(unsigned int i = 0; i < rpcPanelList.size(); i++)
  {
    info() << "module row: " << rpcPanelList[i]->row() << "\t";
    info() << "module column: " << rpcPanelList[i]->col() << endreq;
    info() << "number of fired layers: " << rpcPanelList[i]->firedLayerNum();
    info() << endreq;
    m_treeStruct->firedLayerNum[i] = rpcPanelList[i]->firedLayerNum();
    m_treeStruct->forceTrigger->push_back(rpcPanelList[i]->forceTrigger());
    
    
    /// counts number of modules with hits
    if(m_treeStruct->firedLayerNum[i])
    {
      m_treeStruct->hitRow->push_back(rpcPanelList[i]->row());
      m_treeStruct->hitCol->push_back(rpcPanelList[i]->col());
      m_treeStruct->nLayers->push_back(rpcPanelList[i]->firedLayerNum());
      
      const DaqRpcPanel::RpcStripPtrList& stripList
                                           = rpcPanelList[i]->daqRpcStripList();

      std::vector<int> stripIds;
      for(unsigned int j = 0; j < stripList.size(); j++)
      {
        stripIds.push_back(stripList[j]->channelId().connector());
        info() << "connector: " << stripIds[j];
        info() << endreq;
      }

      m_treeStruct->stripId->push_back(stripIds);

      np++;
    }

    info() << "##### recorded modules: " << m_treeStruct->stripId->size();
    info() << endreq;
    
    for(unsigned int j = 0; j < m_treeStruct->stripId->size(); j++)
    {
      info() << "##### recorded strips: ";
      info() << (*m_treeStruct->stripId)[j].size();
      info() << endreq;
    }
    
    m_treeStruct->fromRot[i] = rpcPanelList[i]->fromRot();
  }
  m_treeStruct->nHitPanels = np;
  
  m_t->Fill();
  
  return StatusCode::SUCCESS;
}
