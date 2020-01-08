#include "iostream"
/*#include "TROOT.h"
using namespace ROOT;
#include "Math/Vector3D.h"
#include "Math/Vector3Dfwd.h"*/
#include "math.h"
#include "globalconstants.h"
using namespace std;

double GetComptonPhotonAngle(const double&);
double ElectronScattering(myVector, unitBasis, double, double);

int main(int argc, char *argv[])
{
	int i;
	int layer;
	double ptEng, eKE, resultantz;
	int instance, nslice;

	/* input parameter check */
	if(argc != 4)
	{
		cout << "Three parameters needed!" << endl;
		cout << "1. incident photon energy in MeV" << endl;
		cout << "2. number of events" << endl;
		cout << "3. number of slices the distance is subdivided into" << endl;
		return 0;
	}
	ptEng = atof(argv[1]);
	instance = atoi(argv[2]);
	nslice = atoi(argv[3]);

	double pathStep = THICKNESS/nslice;

	/* the number of events originated at that particular layer */
	int *originatedLayer = NULL;
	originatedLayer = new int[nslice];

	/* successes layer by layer */
	int *successLBL = NULL;
	successLBL = new int[nslice];
	for(i = 0; i < nslice; i++)
	{
		originatedLayer[i] = 0;
		successLBL[i] = 0;
	}

	/* the ratio of photon energy to electron rest mass */
	const double epsln = ptEng/.511;

	/* cosine of photon angle */
	double cosPtAng, cosEleAng, sinEleAng;

	/* number of electrons that penetrate bakelite successfully */
	int success = 0;

	/* number of electrons scattered backward */
	int backward = 0;

	/* Create seed based on current time */
	srand(time(NULL));
	myVector position;
	unitBasis nextBasis;

	for(i = 0; i < instance; i++)
	{
		/* generate an electron */
		layer = rand()%nslice;
		//cout << layer << " ";

		position.x = 0.;
		position.y = 0.;
		position.z = THICKNESS/(double)nslice*(double)layer;
		originatedLayer[layer]++;

		cosPtAng = GetComptonPhotonAngle(epsln);
		cosEleAng = (1+epsln)*sqrt((1-cosPtAng)/(2+epsln*(epsln+2)*(1-cosPtAng)));
		sinEleAng = sqrt(1-cosEleAng*cosEleAng);
		eKE = .511*epsln*epsln*(1-cosPtAng)/(1+epsln*(1-cosPtAng));
		nextBasis.ex.x = cosEleAng;
		nextBasis.ex.y = 0.;
		nextBasis.ex.z = -1*sinEleAng;
		nextBasis.ey.x = 0.;
		nextBasis.ey.y = 1.;
		nextBasis.ey.z = 0.;
		nextBasis.ez.x = sinEleAng;
		nextBasis.ez.y = 0.;
		nextBasis.ez.z = cosEleAng;
		

		resultantz = ElectronScattering(position, nextBasis, eKE, pathStep);

		if(resultantz >= THICKNESS)
			success++;
		else if(resultantz < 0)
			backward++;
		if((i+1)%100 == 0)
			cout << i+1 << " events have been generated." << endl;
	}

	cout << "Forward percentage: " << (double)success/instance << endl;
	cout << "Backward percentage: " << (double)backward/instance << endl;

	delete [] originatedLayer;
	originatedLayer = NULL;
	delete [] successLBL;
	successLBL = NULL;
	return 0;
}

double GetComptonPhotonAngle(const double &epsln)
{
	double r, ymax, ydist;
	double xrand, yrand;

	while(1)
	{
		r = (double)rand() / ((double)(RAND_MAX)+(double)(1));
		/* x ranges from -1 to +1 */
		xrand = 2*r-1;

		ydist = .5*((1+xrand*xrand)/(1+epsln*(1-xrand))/(1+epsln*(1-xrand)))*(1+epsln*epsln*(1-xrand)*(1-xrand)/(1+xrand*xrand)/(1+epsln*(1-xrand)));

		ymax = 1.;
		r = (double)rand() / ((double)(RAND_MAX)+(double)(1));
		yrand = r*ymax;
		if(yrand < ydist)
			return xrand;
	}
}
