#include "GaudiKernel/DeclareFactoryEntries.h"
#include "NeutronAlg.hpp"

DECLARE_ALGORITHM_FACTORY(NeutronAlg);

DECLARE_FACTORY_ENTRIES(Muon2PassAnalysis)
{
  DECLARE_ALGORITHM(NeutronAlg);
}
