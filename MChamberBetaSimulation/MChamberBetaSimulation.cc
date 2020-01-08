/*******************************************************************
* 2010 Fall @ Daya Bay. This serves as my first Geant4 simulation. *
* This simulates a beta source just on top of the monitor chamber. *
*******************************************************************/

#include "G4RunManager.hh"
#include "G4UImanager.hh"

#include "MChamberConstruction.hh"

int main()
{
	//construct the default run manager
	G4RunManager *runManager = new G4RunManager;
	
	//construct the detector
	G4VUserDetectorConstruction *mChamber = new MChamberConstruction;
	runManager->SetUserInitialization(mChamber);
	
	//list the physics
	
	delete runManager;
	
	return 0;
}
