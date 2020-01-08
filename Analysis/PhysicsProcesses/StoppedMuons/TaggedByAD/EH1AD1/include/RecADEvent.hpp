#ifndef RECADEVENT_HPP
#define RECADEVENT_HPP



class TTree;


class RecADEvent
{
public:

  enum position
  {
    left,
    in,
    right
  };
  
  unsigned int triggerNumber;
  long         triggerTimeInNs;
  float        energy;
  int          energyStatus;
  short        detector;
  float        nPESum;
  float        x;
  float        y;
  float        z;
  
  
  RecADEvent() {};
  RecADEvent(TTree*, int);
  RecADEvent(TTree*, TTree*, int);
  void fillNPESum(TTree*);
  void fillVariables(TTree*);
  void getEntry(TTree*, int);
  void getEntryWithIndex(TTree*, int, int);
  bool isMichelElectron(TTree*, RecADEvent);
  bool isMichel();
  bool isFlasher(TTree*);
  bool isStoppedMuon(TTree*);
  position position2Window(long);
};




#endif
