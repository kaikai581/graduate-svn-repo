#include "GaudiKernel/DeclareFactoryEntries.h"
#include "MuonInducedNeutronAlg.hpp"
#include "MuonTrackAllDetectorsAlg.hpp"
#include "PrintMuonRecSimpleAlg.hpp"
#include "PrintPoolRpcRecAlg.hpp"
#include "ReconValidAlg.hpp"

DECLARE_ALGORITHM_FACTORY(MuonInducedNeutronAlg);
DECLARE_ALGORITHM_FACTORY(MuonTrackAllDetectorsAlg);
DECLARE_ALGORITHM_FACTORY(PrintMuonRecSimpleAlg);
DECLARE_ALGORITHM_FACTORY(PrintPoolRpcRecAlg);
DECLARE_ALGORITHM_FACTORY(ReconValidAlg);

DECLARE_FACTORY_ENTRIES(MuonLookupTable)
{
  DECLARE_ALGORITHM(MuonInducedNeutronAlg);
  DECLARE_ALGORITHM(MuonTrackAllDetectorsAlg);
  DECLARE_ALGORITHM(PrintMuonRecSimpleAlg);
  DECLARE_ALGORITHM(PrintPoolRpcRecAlg);
  DECLARE_ALGORITHM(ReconValidAlg);
}
