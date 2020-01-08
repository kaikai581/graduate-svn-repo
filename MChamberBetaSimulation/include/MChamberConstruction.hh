/*******************************************************************
* This header file contains the dimension of our monitor chamber   *
* prototype.                                                       *
*******************************************************************/

#ifndef MCHAMBERCONSTRUCTION
#define MCHAMBERCONSTRUCTION

class G4LogicalVolume;
class G4VPhysicalVolume;

#include "G4VUserDetectorConstruction.hh"

class MChamberConstruction : public G4VUserDetectorConstruction
{
public:
	MChamberConstruction();
	~MChamberConstruction();
	G4VPhysicalVolume *ExpSetup();
private:
	//logical volumes
	G4LogicalVolume *logUniverse;
	//physical volumes
	G4VPhysicalVolume *physUniverse;
};

#endif
