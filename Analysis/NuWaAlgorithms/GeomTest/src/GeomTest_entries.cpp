#include "GaudiKernel/DeclareFactoryEntries.h"
#include "PrintAdPmtAlg.hpp"
#include "PrintAdPmtInLocalAlg.hpp"
#include "PrintAllAlg.hpp"


DECLARE_ALGORITHM_FACTORY(PrintAdPmtAlg);
DECLARE_ALGORITHM_FACTORY(PrintAdPmtInLocalAlg);
DECLARE_ALGORITHM_FACTORY(PrintAllAlg);

DECLARE_FACTORY_ENTRIES(GeomTest)
{
  DECLARE_ALGORITHM(PrintAdPmtAlg);
  DECLARE_ALGORITHM(PrintAdPmtInLocalAlg);
  DECLARE_ALGORITHM(PrintAllAlg);
}
