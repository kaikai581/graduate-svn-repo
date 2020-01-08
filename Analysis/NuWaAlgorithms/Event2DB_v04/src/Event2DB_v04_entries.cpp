#include "GaudiKernel/DeclareFactoryEntries.h"
#include "Event2DB_v04Alg.hpp"
#include "Event2DB_v041Alg.hpp"
#include "Event2DB_v042Alg.hpp"
#include "Event2DB_v043Alg.hpp"

DECLARE_ALGORITHM_FACTORY(Event2DB_v04Alg);
DECLARE_ALGORITHM_FACTORY(Event2DB_v041Alg);
DECLARE_ALGORITHM_FACTORY(Event2DB_v042Alg);
DECLARE_ALGORITHM_FACTORY(Event2DB_v043Alg);

DECLARE_FACTORY_ENTRIES(Event2DB_v04)
{
  DECLARE_ALGORITHM(Event2DB_v04Alg);
  DECLARE_ALGORITHM(Event2DB_v041Alg);
  DECLARE_ALGORITHM(Event2DB_v042Alg);
  DECLARE_ALGORITHM(Event2DB_v043Alg);
}