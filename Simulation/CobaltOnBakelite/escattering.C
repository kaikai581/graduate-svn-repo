#include "iostream"
#include "math.h"
#include <time.h>
#include "globalconstants.h"
using namespace std;

double GetRandomTheta(double);
double GetRandomPhi();

double ElectronScattering(myVector position, unitBasis currBasis, double eKE, double pathStep)
{
	int forward = 0, backward = 0;
	double theta, phi;
	double gamma;
	double beta;
	double thetarms;
	double lossRate;
	double st, ct, sp, cp;
	bool stop;
	//FILE *of;
	unitBasis nextBasis;

	gamma = 1+eKE/.511;
	beta = sqrt(gamma*gamma-1)/gamma;
	lossRate = DE*EDENSITY/beta/beta*(log(2*511e+3*beta*beta*gamma*gamma/MIP)-beta*beta);
	stop = false;
	while(!stop)
	{
		thetarms = 21*sqrt(pathStep/RADIATIONLENGTH)/(.511*gamma*beta*beta);
		theta = GetRandomTheta(thetarms);
		phi = GetRandomPhi();
		st = sin(theta);
		ct = cos(theta);
		sp = sin(phi);
		cp = cos(phi);
		nextBasis.ez.x = st*cp*currBasis.ex.x+st*sp*currBasis.ey.x+ct*currBasis.ez.x;
		nextBasis.ez.y = st*cp*currBasis.ex.y+st*sp*currBasis.ey.y+ct*currBasis.ez.y;
		nextBasis.ez.z = st*cp*currBasis.ex.z+st*sp*currBasis.ey.z+ct*currBasis.ez.z;
		nextBasis.ex.x = sp*currBasis.ex.x-cp*currBasis.ey.x;
		nextBasis.ex.y = sp*currBasis.ex.y-cp*currBasis.ey.y;
		nextBasis.ex.z = sp*currBasis.ex.z-cp*currBasis.ey.z;
		nextBasis.ey.x = ct*cp*currBasis.ex.x+ct*sp*currBasis.ey.x-st*currBasis.ez.x;
		nextBasis.ey.y = ct*cp*currBasis.ex.y+ct*sp*currBasis.ey.y-st*currBasis.ez.y;
		nextBasis.ey.z = ct*cp*currBasis.ex.z+ct*sp*currBasis.ey.z-st*currBasis.ez.z;
	/* update information to the next step */
		position.x += pathStep*nextBasis.ez.x;
		position.y += pathStep*nextBasis.ez.y;
		position.z += pathStep*nextBasis.ez.z;
		currBasis.ex.x = nextBasis.ex.x;
		currBasis.ex.y = nextBasis.ex.y;
		currBasis.ex.z = nextBasis.ex.z;
		currBasis.ey.x = nextBasis.ey.x;
		currBasis.ey.y = nextBasis.ey.y;
		currBasis.ey.z = nextBasis.ey.z;
		currBasis.ez.x = nextBasis.ez.x;
		currBasis.ez.y = nextBasis.ez.y;
		currBasis.ez.z = nextBasis.ez.z;
		eKE -= lossRate*pathStep;
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
	/*if((i+1)%10 == 0)
		cout << i+1 << " events have been generated." << endl;

	cout << "Forward percentage: " << (double)forward/INSTANCE << endl;
	cout << "Backward percentage: " << (double)backward/INSTANCE << endl;
	of = fopen("escatResult", "w");
	fprintf(of, "Forward percentage: %lf\n", (double)forward/INSTANCE);
	fprintf(of, "Backward percentage: %lf\n", (double)backward/INSTANCE);
	fclose(of);*/

	return position.z;
}

double GetRandomTheta(double thetarms)
{
	double r, ymax, ydist;
	double xrand, yrand;

	while(1)
	{
		r = (double)rand() / ((double)(RAND_MAX)+(double)(1));
		xrand = r*M_PI;
		//cout << r << " ";

		ydist = 2*xrand*exp(-1*xrand*xrand/thetarms/thetarms)/thetarms/thetarms;

		ymax = sqrt(2)*exp(-0.5)/thetarms;
		r = (double)rand() / ((double)(RAND_MAX)+(double)(1));
		//cout << r << endl;
		yrand = r*ymax;
		if(yrand < ydist)
			return xrand;
	}
}

double GetRandomPhi()
{
	return ((double)rand() / ((double)(RAND_MAX)+(double)(1)))*2*M_PI;
}
