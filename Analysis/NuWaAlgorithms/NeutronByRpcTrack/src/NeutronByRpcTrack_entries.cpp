#include "GaudiKernel/DeclareFactoryEntries.h"
#include "MuonTreeAlg.hpp"

DECLARE_ALGORITHM_FACTORY(MuonTreeAlg);

DECLARE_FACTORY_ENTRIES(NeutronByRpcTrack)
{
  DECLARE_ALGORITHM(MuonTreeAlg);
}
