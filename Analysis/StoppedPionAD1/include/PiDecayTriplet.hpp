#ifndef PIDECAYTRIPLET_HPP
#define PIDECAYTRIPLET_HPP




class PiDecayTriplet
{
public:

  /// p(i) means particile i. The time order is p1->p2->p3
  /// p1 should be pion, p2 muon and p3 electron.
  float pe_p1;
  float pe_p2;
  float pe_p3;

  float t1;
  float t2;
  float t3;
};
