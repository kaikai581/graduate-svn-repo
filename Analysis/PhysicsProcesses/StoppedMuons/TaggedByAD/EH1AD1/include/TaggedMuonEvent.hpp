#ifndef TAGGEDMUONEVENT_HPP
#define TAGGEDMUONEVENT_HPP



class TTree;



class TaggedMuonEvent
{
public:
  int hitRPC;
  int triggerNumberAD[4];
  int triggerNumberRPC;
  
  
  TaggedMuonEvent() {};
  TaggedMuonEvent(TTree*, int);
  void fillVariables(TTree*, int);
};





#endif
