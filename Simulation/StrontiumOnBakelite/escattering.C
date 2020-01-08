#include "iostream"
#include "math.h"
#include <time.h>
#include <stdlib.h>
using namespace std;

#define INSTANCE 1000
#define NSLICE 2000

//#define EENERGY 2.28 //electron energy in MeV
//#define EENERGY 1
#define EENERGY 1.769
#define THICKNESS .2 //bakelite thickness in cm
#define PATHSTEP THICKNESS/NSLICE
#define RADIATIONLENGTH 28.6 //radiation length in cm
#define MIP 72.4 //mean ionization potential
#define DE 5.0989e-25 //fixed constants and electron properties in MeV-cm^2   cf. Fernow p.38
#define EDENSITY 3.48e+23 //electron density of bakelite

double GetRandomTheta(double);
double GetRandomPhi();

typedef struct
{
	double x;
	double y;
	double z;
}myVector;

int main()
{
	int i, forward = 0, backward = 0;
	double theta, phi;
	double gamma;
	double beta;
	double thetarms;
	double eKE; //electron kinetic energy
	double lossRate;
	double st, ct, sp, cp;
	bool stop;
	FILE *of;
	myVector position, currunit[3], nextunit[3];

	srand(time(NULL)); /* Create seed based on current time */
	for(i = 0; i < INSTANCE; i++)
	{
		/* Initialize the electron condition */
		position.x = 0;
		position.y = 0;
		position.z = 0;
		currunit[0].x = 1;
		currunit[0].y = 0;
		currunit[0].z = 0;
		currunit[1].x = 0;
		currunit[1].y = 1;
		currunit[1].z = 0;
		currunit[2].x = 0;
		currunit[2].y = 0;
		currunit[2].z = 1;
		eKE = EENERGY;
		gamma = 1+eKE/.511;
		beta = sqrt(gamma*gamma-1)/gamma;
		lossRate = DE*EDENSITY/beta/beta*(log(2*511e+3*beta*beta*gamma*gamma/MIP)-beta*beta);
		stop = false;
		while(!stop)
		{
			thetarms = 21*sqrt(PATHSTEP/RADIATIONLENGTH)/(.511*gamma*beta*beta);
			theta = GetRandomTheta(thetarms);
			phi = GetRandomPhi();
			st = sin(theta);
			ct = cos(theta);
			sp = sin(phi);
			cp = cos(phi);
			nextunit[2].x = st*cp*currunit[0].x+st*sp*currunit[1].x+ct*currunit[2].x;
			nextunit[2].y = st*cp*currunit[0].y+st*sp*currunit[1].y+ct*currunit[2].y;
			nextunit[2].z = st*cp*currunit[0].z+st*sp*currunit[1].z+ct*currunit[2].z;
			nextunit[0].x = sp*currunit[0].x-cp*currunit[1].x;
			nextunit[0].y = sp*currunit[0].y-cp*currunit[1].y;
			nextunit[0].z = sp*currunit[0].z-cp*currunit[1].z;
			nextunit[1].x = ct*cp*currunit[0].x+ct*sp*currunit[1].x-st*currunit[2].x;
			nextunit[1].y = ct*cp*currunit[0].y+ct*sp*currunit[1].y-st*currunit[2].y;
			nextunit[1].z = ct*cp*currunit[0].z+ct*sp*currunit[1].z-st*currunit[2].z;

			/* update information to the next step */
			position.x += PATHSTEP*nextunit[2].x;
			position.y += PATHSTEP*nextunit[2].y;
			position.z += PATHSTEP*nextunit[2].z;
			currunit[0].x = nextunit[0].x;
			currunit[0].y = nextunit[0].y;
			currunit[0].z = nextunit[0].z;
			currunit[1].x = nextunit[1].x;
			currunit[1].y = nextunit[1].y;
			currunit[1].z = nextunit[1].z;
			currunit[2].x = nextunit[2].x;
			currunit[2].y = nextunit[2].y;
			currunit[2].z = nextunit[2].z;
			eKE -= lossRate*PATHSTEP;
			gamma = 1+eKE/.511;
			beta = sqrt(gamma*gamma-1)/gamma;
			lossRate = DE*EDENSITY/beta/beta*(log(2*511e+3*beta*beta*gamma*gamma/MIP)-beta*beta);
			if(position.z >= 0.2)
			{
				forward++;
				stop = true;
				continue;
			}
			if(position.z < 0)
			{
				backward++;
				stop = true;
				continue;
			}
			if(eKE <= 0)
			{
				stop = true;
				continue;
			}
			//cout << theta << " " << position.z << endl;
		}
		if((i+1)%10 == 0)
			cout << i+1 << " events have been generated." << endl;
	}

	cout << "Forward percentage: " << (double)forward/INSTANCE << endl;
	cout << "Backward percentage: " << (double)backward/INSTANCE << endl;
	of = fopen("escatResult", "w");
	fprintf(of, "Forward percentage: %lf\n", (double)forward/INSTANCE);
	fprintf(of, "Backward percentage: %lf\n", (double)backward/INSTANCE);
	fclose(of);

	return 0;
}

double GetRandomTheta(double thetarms)
{
	double r, ymax, ydist;
	double xrand, yrand;

	while(1)
	{
		r = (double)rand() / ((double)(RAND_MAX)+(double)(1));
		xrand = r*M_PI;

		ydist = 2*xrand*exp(-1*xrand*xrand/thetarms/thetarms)/thetarms/thetarms;

		ymax = sqrt(2)*exp(-0.5)/thetarms;
		r = (double)rand() / ((double)(RAND_MAX)+(double)(1));
		yrand = r*ymax;
		if(yrand < ydist)
			return xrand;
	}
}

double GetRandomPhi()
{
	return ((double)rand() / ((double)(RAND_MAX)+(double)(1)))*2*M_PI;
}
