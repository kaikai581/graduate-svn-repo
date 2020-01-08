#include "GaudiKernel/DeclareFactoryEntries.h"
#include "FirstPassAlg.hpp"
#include "Muon2DBAlg.hpp"

DECLARE_ALGORITHM_FACTORY(FirstPassAlg);
DECLARE_ALGORITHM_FACTORY(Muon2DBAlg);

DECLARE_FACTORY_ENTRIES(Muon2DB)
{
  DECLARE_ALGORITHM(FirstPassAlg);
  DECLARE_ALGORITHM(Muon2DBAlg);
}
