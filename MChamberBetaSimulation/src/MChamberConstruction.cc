/*******************************************************************
* 2010 Fall @ Daya Bay. This serves as my first Geant4 simulation. *
*******************************************************************/

#include "MChamberConstruction.hh"

#include "G4Material.hh"
#include "G4Element.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "globals.hh"

MChamberConstruction::MChamberConstruction()
: logUniverse(0), physUniverse(0)
{;}

MChamberConstruction::~MChamberConstruction()
{
}

G4VPhysicalVolume * MChamberConstruction::ExpSetup()
{
	//material: bakelite
	G4double a;
	G4double z;
	G4double fractionmass;
	G4String name;
	G4String symbol;
	G4int ncomponents;
	G4double bakeliteDensity = 1.42*g/cm3;
	//H
	a = 1.00794*g/mole;
	G4Element *elH = new G4Element(name="Hydrogen", symbol="H", z=1., a);
	//C
	a = 12.011*g/mole;
	G4Element *elC = new G4Element(name="Carbon", symbol="C", z=6., a);
	//O
	a = 15.9994*g/mole;
	G4Element *elO = new G4Element(name="Oxygen", symbol="O", z=8., a);
	
	G4Material *bakelite = new G4Material(name="Bakelite", bakeliteDensity, ncomponents=3);
	bakelite->AddElement(elH, fractionmass=5.7441*perCent);
	bakelite->AddElement(elC, fractionmass=77.4591*perCent);
	bakelite->AddElement(elO, fractionmass=16.7968*perCent);
	
	//detector geometry
	G4double universeX = 20*cm;
	G4double universeY = 30*cm;
	G4double universeZ = 2*mm;
	
	G4Box *universeBox = new G4Box("universeBox", universeX, universeY, universeZ);
	logUniverse = new G4LogicalVolume(universeBox, bakelite, "logUniverse");
	physUniverse = new G4PVPlacement(0, G4ThreeVector(), logUniverse, "physUniverse", 0, false, 0);
	return physUniverse;
}
