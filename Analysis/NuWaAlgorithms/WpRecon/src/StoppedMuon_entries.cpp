#include "AESTest.hpp"
#include "GaudiKernel/DeclareFactoryEntries.h"
#include "MuonLookBack.hpp"
#include "MuonTaggedByAD.hpp"
#include "ElectronLookBack.hpp"

DECLARE_ALGORITHM_FACTORY(AESTest);
DECLARE_ALGORITHM_FACTORY(MuonTaggedByAD);
DECLARE_ALGORITHM_FACTORY(ElectronLookBack);
DECLARE_ALGORITHM_FACTORY(MuonLookBack);

DECLARE_FACTORY_ENTRIES(StoppedMuon)
{
  DECLARE_ALGORITHM(AESTest);
  DECLARE_ALGORITHM(MuonTaggedByAD);
  DECLARE_ALGORITHM(ElectronLookBack);
  DECLARE_ALGORITHM(MuonLookBack);
}
