#include <iostream>
#include "TMath.h"

using namespace std;

/// input energy in GeV
void MeanEnergyLoss(double E = 1)
{
  if(E < 1e-6)
  {
    cerr << "input energy too small" << endl;
    return;
  }
  /// all energy in MeV
  E *= 1000;
  double Mmu = 105.6583715;
  double Me = .511;
  double n = 6e23*5*.86;
  double hbar = 6.582e-16*1e-6;
  double c = 3e10;
  // mean ionization potential
  double mip = 1e-4;
  // beta squared
  double beta2 = 1-(Mmu/E)*(Mmu/E);
  double dEdx;
  dEdx = 4*TMath::Pi()/Me*n/beta2*(hbar*c/137)*(hbar*c/137)*(log(2*Me*beta2/mip/(1-beta2))-beta2);
  cout << dEdx << endl;
  
  double gamma2 = 1/(1-beta2);
  double Tmax = 2*Me*beta2*gamma2/(1+2*sqrt(gamma2)*Me/Mmu+(Me/Mmu)*(Me/Mmu));
  dEdx = 0.307*.5/beta2*(.5*log(2*Me*beta2*gamma2*Tmax/mip/mip)-beta2);
  cout << dEdx << endl;
}
