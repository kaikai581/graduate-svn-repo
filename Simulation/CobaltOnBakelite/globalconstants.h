#ifndef GLOBALCONSTANTS_H
#define GLOBALCONSTANTS_H

#include <stdlib.h>
//#define NSLICE 2000 //number of slices into which the z axis is devided
//#define INSTANCE 1000 //number of events generated

#define THICKNESS .2 //bakelite thickness in cm
//#define PATHSTEP THICKNESS/NSLICE
#define MDENSITY 1.4 //mass density of bakelite
//#define EDENSITY 3.48e+23 //electron density of bakelite
#define EDENSITY 4.4e+23 //electron density of bakelite
#define DE 5.0989e-25 //fixed constants and electron properties in MeV-cm^2   cf. Fernow p.38
#define MIP 72.4 //mean ionization potential
#define RADIATIONLENGTH 28.6 //radiation length in cm

typedef struct
{
	double x;
	double y;
	double z;
}myVector;

typedef struct
{
	myVector ex;
	myVector ey;
	myVector ez;
}unitBasis;

#endif
