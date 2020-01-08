/*
 * 
 * This program generates a ramdom muon track in AD IAV and tries to find
 * an inscribed cylinder.
 * For the equations, see the group meeting report on Jan. 31, 2013.
 * circa January 2013 @ UH
 * 
 */


#include "Math/AxisAngle.h"

#include "Math/EulerAngles.h"

#include "Math/Point2D.h"

#include "Math/Point3D.h"

#include "Math/Transform3D.h"

#include "Math/Vector3D.h"

#include "TAxis.h"

#include "TFile.h"

#include "TGraph.h"

#include "TMath.h"

#include "TRandom3.h"

#include "TTree.h"

#include "tclap/CmdLine.h"

#define RADIUSIAV 1500.

#define RADIUSTR 1000.

 
using namespace std;


namespace point {
  enum IntersectType {
    kNone,
    kTop,
    kSide,
    kBot,
  };
}


namespace track {
  enum IntersectType {
    kNone,
    kTopBot,
    kTopSide,
    kSideBot,
    kSideSide
  };
}


class TrackInfo
{

public:
  
  /// point in the RPC plane
  ROOT::Math::XYZPoint pRpc;
  
  /// muon direction
  ROOT::Math::XYZVector dirVec;
  
  /// point of the track in xy plane
  ROOT::Math::XYZPoint pXYPlane;
  
  /// length of pXYPlane
  double originSeparation;

  /// line parameter of intersecting points
  double tCylinderIn, tCylinderOut;
  
  double tIavIn, tIavOut;
  ROOT::Math::XYZPoint pIavIn, pIavOut;
  
  /// track length
  double trackLength;

  /// point intersection type, if any
  point::IntersectType pointIntersectTypeIn, pointIntersectTypeOut;

  /// track intersection type, if any
  track::IntersectType trackIntersectType;
  
  /// data for the intersection curve of two infinite cylinders
  vector< ROOT::Math::XYPoint > intersectCurve;
  
  /// if there are two separate contours on the track cylinder
  /// indicating first posibility of inscribability
  bool twoContour;
  
  /// if inscribing a cylinder is possible by taking into acount the
  /// top and bottom plane of IAV
  bool inscribable;
  
  /// record the tangent point of the inscribing cylinder in IAV coordinate
  ROOT::Math::XYZPoint pInsCylTopIavCoord, pInsCylBotIavCoord;
  
  /// the z value of the upper and lower planes of the inscribing cylinder
  /// in track coordinate
  double zInsCylTopTrackCoord, zInsCylBotTrackCoord;

public:

  TrackInfo();

};


TrackInfo::TrackInfo() : twoContour(false), inscribable(false)
{
}


/// given a circle with center at (0,0,zTrackCoord) in track coordinate, 
/// find if this circle is completely contained in the IAV
bool circleInsideIav(TrackInfo& ti, double zTrackCoord)
{
  int nstep = 360;
  
  ROOT::Math::AxisAngle rotTrack(ROOT::Math::XYZVector(0,0,1).Cross(-ti.dirVec),(-ti.dirVec).Theta());
  
  ROOT::Math::Transform3D track2Iav(rotTrack, (ROOT::Math::XYZVector)ti.pXYPlane);
  
  for(int i = 0; i < nstep; i++)
  {
    double th = i*TMath::TwoPi()/nstep;
    ROOT::Math::XYZPoint pCircleTrackCoord(RADIUSTR*cos(th), RADIUSTR*sin(th), zTrackCoord);
    
    double thisZ = track2Iav(pCircleTrackCoord).Z();
    
    if(thisZ > RADIUSIAV || thisZ < -RADIUSIAV) return false;
  }
  
  return true;
}


void findTangentPoint(TrackInfo& ti)
{
  
  /// three Euler angles for AD isometric transformation
  //double phi = ti.dirVec.Phi()+TMath::PiOver2()*3;
  
  //double theta = ti.dirVec.Theta();
  
  //double psi = 0;
  
  /// construct rotation matrix with ROOT's built-in library
  
  ROOT::Math::XYZVector target = -ti.dirVec;
  //ROOT::Math::XYZVector target(0,1,0);
  //target /= sqrt(target.Mag2());
  
  ROOT::Math::XYZVector axis = -ROOT::Math::XYZVector(0,0,1).Cross(target);
  
  ROOT::Math::AxisAngle rotIav(axis, target.Theta());
  
  ROOT::Math::AxisAngle rotTrack = rotIav.Inverse();
  
  //ROOT::Math::XYZVector resVec = rotIav*ROOT::Math::XYZVector(0,0,1);
  
  //cout << endl << target.X() << " " << target.Y() << " " << target.Z() << endl;
  
  //cout << resVec.X() << " " << resVec.Y() << " " << resVec.Z() << endl << endl;
  
  ROOT::Math::XYZVector transIav = -(rotIav*((ROOT::Math::XYZVector)ti.pXYPlane));
  //ROOT::Math::XYZVector transIav = -(rotIav*(ROOT::Math::XYZVector(500/sqrt(2),500/sqrt(2),0)));
  
  /// extract the rotation matrix elements
  ROOT::Math::XYZVector e1(1,0,0);
  
  ROOT::Math::XYZVector e2(0,1,0);
  
  ROOT::Math::XYZVector e3(0,0,1);
  
  double a11 = e1.Dot(rotIav.Inverse()*e1);
  
  double a12 = e1.Dot(rotIav.Inverse()*e2);
  
  double a13 = e1.Dot(rotIav.Inverse()*e3);
  
  double a21 = e2.Dot(rotIav.Inverse()*e1);
  
  double a22 = e2.Dot(rotIav.Inverse()*e2);
  
  double a23 = e2.Dot(rotIav.Inverse()*e3);
  
  double s1 = (rotIav.Inverse()*(-transIav)).X();
  
  double s2 = (rotIav.Inverse()*(-transIav)).Y();
  
  /// draw the solution curve
  int nstep = 360;
  
  /// if the discriminant is ever negative, a closed loop is formed
  /// and a small cylinder can not be inscribed
  ti.twoContour = true;
  
  ///// containers for upper and lower curves for finding the tangent points
  //vector< ROOT::Math::XYPoint > uppercurve;
  
  //vector< ROOT::Math::XYPoint > lowercurve;
  
  /// find the minimum of the upper curve and
  /// the maximum of the lower curve and the corresponding theta values
  double zupmin, zlowmax;
  
  double xup, xlow;
  
  bool firstRealSolution = true;
  
  /// for the equation, see the group meeting report on Jan. 31, 2013
  for(int i = 0; i < nstep; i++)
  {
    
    double x = -TMath::Pi()+TMath::TwoPi()/nstep*i;
    
    double R = RADIUSIAV;
    
    double r = RADIUSTR;
    
    double x1 = r*cos(x), x2 = r*sin(x);
    
    double a2 = a13*a13+a23*a23;
    
    double rcoor1 = a11*x1+a12*x2+s1, rcoor2 = a21*x1+a22*x2+s2;
    
    double a1 = 2*(a13*rcoor1+a23*rcoor2);
    
    double a0 = rcoor1*rcoor1+rcoor2*rcoor2-R*R;
    
    double D = a1*a1-4*a0*a2;
    
    if(D > 0) {
      
      double yup = 1./2/a2*(-a1+sqrt(D));
      
      double ylow = 1./2/a2*(-a1-sqrt(D));
      
      ti.intersectCurve.push_back(ROOT::Math::XYPoint(x,yup));
      
      ti.intersectCurve.push_back(ROOT::Math::XYPoint(x,ylow));
      
      
      if(firstRealSolution) /// initialize
      {
        
        zupmin = yup;
        
        zlowmax = ylow;
        
        xup = x;
        
        xlow = x;
        
        firstRealSolution = false;
        
      }
      
      else {
        
        if(yup < zupmin) {
          
          zupmin = yup;
          
          xup = x;
          
        }
        
        if(ylow > zlowmax) {
          
          zlowmax = ylow;
          
          xlow = x;
          
        }
        
      }
      
    }
    
    else ti.twoContour = false;
    
  }
  
  cout << zupmin - zlowmax << " " << boolalpha << ti.twoContour << endl;
  
  /// now consider the intersection is on the top/bottom IAV plane
  ROOT::Math::XYZPoint pTrackTop, pTrackDown;
  
  pTrackTop = ti.pRpc - (ti.pRpc.Z()-RADIUSIAV)/ti.dirVec.Z()*ti.dirVec;
  
  pTrackDown = ti.pRpc - (ti.pRpc.Z()+RADIUSIAV)/ti.dirVec.Z()*ti.dirVec;
  
  ROOT::Math::XYZVector ez(0,0,1);
  
  ROOT::Math::XYZVector vPerp = ez.Cross(ti.dirVec);
  
  /// normalize the perpendicular vector
  vPerp /= sqrt(vPerp.Mag2());
  
  ROOT::Math::XYZVector vAlong = vPerp.Cross(ez);
  
  double tiltAng = (-ti.dirVec).Theta();
  
  ROOT::Math::XYZPoint pTangentTopIavCoord, pTangentBotIavCoord;
  
  pTangentTopIavCoord = pTrackTop+(RADIUSTR/cos(tiltAng))*vAlong;
  
  pTangentBotIavCoord = pTrackDown-(RADIUSTR/cos(tiltAng))*vAlong;

  
  /// check if zupmin is lower than the z of the top IAV plane
  /// and zlowmax is higher than the z of the bottom IAV plane
  ROOT::Math::XYZPoint pUpMinTrackCoord(RADIUSTR*cos(xup), RADIUSTR*sin(xup), zupmin);
  
  ROOT::Math::XYZPoint pLowMaxTrackCoord(RADIUSTR*cos(xlow), RADIUSTR*sin(xlow), zlowmax);
  
  //cout << "min point " << pUpMinTrackCoord.X() << " " << pUpMinTrackCoord.Y() << " " << pUpMinTrackCoord.Z() << endl;
  
  ROOT::Math::XYZPoint pUpMinIavCoord = rotTrack*(ROOT::Math::XYZVector)pUpMinTrackCoord + ti.pXYPlane;
  
  //cout << "min point in IAV coordinate: " << endl << pUpMinIavCoord.Z() << " " << sqrt(pUpMinIavCoord.X()*pUpMinIavCoord.X()+pUpMinIavCoord.Y()*pUpMinIavCoord.Y()) << endl;
  
  ROOT::Math::Transform3D track2Iav(rotTrack, (ROOT::Math::XYZVector)ti.pXYPlane);
  ROOT::Math::Transform3D iav2Track(rotIav, transIav);
  
  pUpMinIavCoord = track2Iav(pUpMinTrackCoord);
  
  ROOT::Math::XYZPoint pLowMaxIavCoord = track2Iav(pLowMaxTrackCoord);
  
  //cout << "using ROOT Transform3D: " << endl << pUpMinIavCoord.Z() << " " << sqrt(pUpMinIavCoord.X()*pUpMinIavCoord.X()+pUpMinIavCoord.Y()*pUpMinIavCoord.Y()) << endl;
  
  //cout << "using ROOT Transform3D: " << endl << pLowMaxIavCoord.Z() << " " << sqrt(pLowMaxIavCoord.X()*pLowMaxIavCoord.X()+pLowMaxIavCoord.Y()*pLowMaxIavCoord.Y()) << endl;
  
  /// get top and bottom tangent points in track coordinate system
  ROOT::Math::XYZPoint pTangentTopTrackCoord = iav2Track(pTangentTopIavCoord);
  
  ROOT::Math::XYZPoint pTangentBotTrackCoord = iav2Track(pTangentBotIavCoord);

  
  if(ti.twoContour) {
    //cout << endl << "two contours on the track cylinder " <<  zupmin << " " << pTangentTopIavCoord.X() << " " << pTangentTopIavCoord.Y() << " " << pTangentTopIavCoord.Z() << endl;
    
    //cout << "inside? " << boolalpha << (sqrt(pTangentTopIavCoord.X()*pTangentTopIavCoord.X()+pTangentTopIavCoord.Y()*pTangentTopIavCoord.Y()) < 1500) << endl;
    
    //cout << "z obtained by another way: " << (rotIav*pTangentTopIavCoord+transIav).X() << " " << (rotIav*pTangentTopIavCoord+transIav).Y() << " " << (rotIav*pTangentTopIavCoord+transIav).Z() << endl << endl;
    
    bool topTangentInside = (sqrt(pTangentTopIavCoord.X()*pTangentTopIavCoord.X()+pTangentTopIavCoord.Y()*pTangentTopIavCoord.Y()) < RADIUSIAV);
    
    bool botTangentInside = (sqrt(pTangentBotIavCoord.X()*pTangentBotIavCoord.X()+pTangentBotIavCoord.Y()*pTangentBotIavCoord.Y()) < RADIUSIAV);
    
    bool topLoopInside = (pUpMinIavCoord.Z() < RADIUSIAV);
    
    bool botLoopInside = (pLowMaxIavCoord.Z() > -RADIUSIAV);
    
    bool topInscribable, botInscribable;
    
    bool upCircleInsideIav = circleInsideIav(ti, zupmin);
    
    bool lowCircleInsideIav = circleInsideIav(ti, zlowmax);
    
    /// deal with top
    if(topTangentInside && topLoopInside) {
      //ti.pInsCylTopIavCoord = (pTangentTopTrackCoord.Z() < pUpMinTrackCoord.Z()) ? pTangentTopIavCoord : pUpMinIavCoord;
      if(pTangentTopTrackCoord.Z() < pUpMinTrackCoord.Z()) {
        ti.pInsCylTopIavCoord = pTangentTopIavCoord;
        cout << "tangent condition 1 top " << pTangentTopIavCoord.Z() << endl;
      }
      else {
        ti.pInsCylTopIavCoord = pUpMinIavCoord;
        cout << "tangent condition 1 side " << sqrt(pUpMinIavCoord.X()*pUpMinIavCoord.X()+pUpMinIavCoord.Y()*pUpMinIavCoord.Y()) << endl;
      }
      
      topInscribable = (upCircleInsideIav ? true : false);
    }
    
    if(topTangentInside && !topLoopInside) {
      ti.pInsCylTopIavCoord = pTangentTopIavCoord;
      cout << "tangent condition 2 " << pTangentTopIavCoord.Z() << endl;
      
      topInscribable = true;
    }
    
    if(!topTangentInside && topLoopInside) {
      ti.pInsCylTopIavCoord = pUpMinIavCoord;
      cout << "tangent condition 3 " << sqrt(pUpMinIavCoord.X()*pUpMinIavCoord.X()+pUpMinIavCoord.Y()*pUpMinIavCoord.Y()) << endl;
      
      topInscribable = (upCircleInsideIav ? true : false);
    }
    
    if(!topTangentInside && !topLoopInside) topInscribable = false;
    
    /// deal with bottom
    if(botTangentInside && botLoopInside) {
      ti.pInsCylBotIavCoord = (pTangentBotTrackCoord.Z() > pLowMaxTrackCoord.Z()) ? pTangentBotIavCoord : pLowMaxIavCoord;
      
      botInscribable = (lowCircleInsideIav ? true : false);
    }
    
    if(botTangentInside && !botLoopInside) {
      ti.pInsCylBotIavCoord = pTangentBotIavCoord;
      
      botInscribable = true;
    }
    
    if(!botTangentInside && botLoopInside) {
      ti.pInsCylBotIavCoord = pLowMaxIavCoord;
      
      botInscribable = (lowCircleInsideIav ? true : false);
    }
    
    if(!botTangentInside && !botLoopInside) botInscribable = false;
    
    ti.zInsCylTopTrackCoord = iav2Track(ti.pInsCylTopIavCoord).Z();
    
    ti.zInsCylBotTrackCoord = iav2Track(ti.pInsCylBotIavCoord).Z();
    
    ti.inscribable = topInscribable && botInscribable && (ti.zInsCylTopTrackCoord > ti.zInsCylBotTrackCoord);
    
  }
  
}


void getTrackPointInXYPlane(TrackInfo& ti)
{
  double scale = -ti.pRpc.Z()/ti.dirVec.Z();
  
  ti.pXYPlane = ti.pRpc + scale*ti.dirVec;
  
  ti.originSeparation = sqrt(ti.pXYPlane.Mag2());
}


bool intersectAd(TrackInfo& ti)
{
  
  /// set initial values for intersection type
  ti.tCylinderIn = -1;
  
  ti.tCylinderOut = -1;
  
  ti.tIavIn = -1;
  
  ti.tIavOut = -1;
  
  ti.trackIntersectType = track::kNone;
  
  /// solve the equation for line parameter t
  
  /// t^2 coefficient
  double a2 = ti.dirVec.X()*ti.dirVec.X()+ti.dirVec.Y()*ti.dirVec.Y();
  
  /// t^1 coefficient
  double a1 = 2*(ti.pRpc.X()*ti.dirVec.X()+ti.pRpc.Y()*ti.dirVec.Y());
  
  /// t^0 coefficient
  double a0 = ti.pRpc.X()*ti.pRpc.X()+ti.pRpc.Y()*ti.pRpc.Y();
  
  a0 -= RADIUSIAV*RADIUSIAV;
  
  /// discriminant of the t quadratic equation
  double disc = a1*a1-4*a2*a0;
  
  if(disc < 0) {
    //cout << "first return" << endl;
    return false;
  }
  
  ti.tCylinderIn = 1./2/a2*(-a1-sqrt(disc));
  
  ti.tCylinderOut = 1./2/a2*(-a1+sqrt(disc));
  
  ROOT::Math::XYZPoint pin = ti.pRpc+ti.tCylinderIn*ti.dirVec;

  ROOT::Math::XYZPoint pout = ti.pRpc+ti.tCylinderOut*ti.dirVec;
  
  //cout << "(" << pin.X() << "," << pin.Y() << "," << pin.Z() << ")" << endl;
  
  //cout << pin.X()*pin.X()+pin.Y()*pin.Y() << endl;
  
  //cout << "(" << pout.X() << "," << pout.Y() << "," << pout.Z() << ")" << endl;
  
  //cout << pout.X()*pout.X()+pout.Y()*pout.Y() << endl;
  
  enum IntersectZone {
    kAbove,
    kIn,
    kBelow
  };
  
  IntersectZone zoneIn, zoneOut;
  
  if(pin.Z() >= RADIUSIAV) zoneIn = kAbove;
  else if(pin.Z() < RADIUSIAV && pin.Z() >= -RADIUSIAV) zoneIn = kIn;
  else zoneIn = kBelow;
  
  if(pout.Z() >= RADIUSIAV) zoneOut = kAbove;
  else if(pout.Z() < RADIUSIAV && pout.Z() >= -RADIUSIAV) zoneOut = kIn;
  else zoneOut = kBelow;
  
  if(zoneIn == kAbove && zoneOut == kIn) {
    ti.pointIntersectTypeIn = point::kTop;
    ti.pointIntersectTypeOut = point::kSide;
    ti.trackIntersectType = track::kTopSide;
    
    ti.tIavIn = -(ti.pRpc.Z()-RADIUSIAV)/ti.dirVec.Z();
    
    ti.tIavOut = ti.tCylinderOut;
    
    //ROOT::Math::XYZPoint pIavTop = ti.pRpc+ti.tIavIn*ti.dirVec;
    
    //cout << "(" << pIavTop.X() << "," << pIavTop.Y() << "," << pIavTop.Z() << ")" << endl;
  
    //cout << pIavTop.X()*pIavTop.X()+pIavTop.Y()*pIavTop.Y() << endl;
  }
  else if(zoneIn == kAbove && zoneOut == kBelow) {
    ti.pointIntersectTypeIn = point::kTop;
    ti.pointIntersectTypeOut = point::kBot;
    ti.trackIntersectType = track::kTopBot;
    
    ti.tIavIn = -(ti.pRpc.Z()-RADIUSIAV)/ti.dirVec.Z();
    
    ti.tIavOut = -(ti.pRpc.Z()+RADIUSIAV)/ti.dirVec.Z();
    
    //ROOT::Math::XYZPoint pIavTop = ti.pRpc+ti.tIavIn*ti.dirVec;
    
    //ROOT::Math::XYZPoint pIavBot = ti.pRpc+ti.tIavOut*ti.dirVec;
    
    //cout << "(" << pIavBot.X() << "," << pIavBot.Y() << "," << pIavBot.Z() << ")" << endl;
    
    //cout << pIavBot.X()*pIavBot.X()+pIavBot.Y()*pIavBot.Y() << endl;
  }
  else if(zoneIn == kIn && zoneOut == kIn) {
    ti.pointIntersectTypeIn = point::kSide;
    ti.pointIntersectTypeOut = point::kSide;
    ti.trackIntersectType = track::kSideSide;
    
    ti.tIavIn = ti.tCylinderIn;
    
    ti.tIavOut = ti.tCylinderOut;
    
    //cout << "side-side" << endl;
  }
  else if(zoneIn == kIn && zoneOut == kBelow) {
    ti.pointIntersectTypeIn = point::kSide;
    ti.pointIntersectTypeOut = point::kBot;
    ti.trackIntersectType = track::kSideBot;
    
    //cout << "side-bottom" << endl;
    
    ti.tIavIn = ti.tCylinderIn;
    
    ti.tIavOut = -(ti.pRpc.Z()+RADIUSIAV)/ti.dirVec.Z();
    
    //ROOT::Math::XYZPoint pIavBot = ti.pRpc+ti.tIavOut*ti.dirVec;
    
    //cout << "(" << pIavBot.X() << "," << pIavBot.Y() << "," << pIavBot.Z() << ")" << endl;
  
    //cout << pIavBot.X()*pIavBot.X()+pIavBot.Y()*pIavBot.Y() << endl;
  }
  else {
    ti.pointIntersectTypeIn = point::kNone;
    ti.pointIntersectTypeOut = point::kNone;
    ti.trackIntersectType = track::kNone;
    //cout << "second return" << endl;
    return false;
  }
  
  ti.pIavIn = ti.pRpc+ti.tIavIn*ti.dirVec;
  
  ti.pIavOut = ti.pRpc+ti.tIavOut*ti.dirVec;
  
  ti.trackLength = sqrt((ti.pIavIn-ti.pIavOut).Mag2());
  
  return true;
}


void loopEvent(TrackInfo& ti, bool& validEvent)
{
  
  /// generate a point randomly in the RPC plane
  TRandom3 ranP(0);
  
  ti.pRpc.SetX(ranP.Uniform(-6000,6000));
  
  ti.pRpc.SetY(ranP.Uniform(-6000,6000));
  
  ti.pRpc.SetZ(5250);
  
  double dirX, dirY, dirZ;
  
  ranP.Sphere(dirX, dirY, dirZ, 1);
  
  ti.dirVec.SetXYZ(dirX, dirY, dirZ);
  
  /// always point the direction downward
  if(dirZ > 0) ti.dirVec *= -1;
  
  getTrackPointInXYPlane(ti);
  
  validEvent = intersectAd(ti);
  
  if(validEvent) findTangentPoint(ti);
}


int main(int argc, char** argv)
{
  
  /// parsing command line arguments
  TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
  
  TCLAP::ValueArg<int> nEvtArg("n", "number",
  "Number of events", false, -1, "integer");
  
  cmd.add(nEvtArg);
  
  cmd.parse(argc, argv);
  
  int nEvt = nEvtArg.getValue();
  
  TFile fout("output/simtrack.root", "RECREATE");
  
  fout.mkdir("graph");
  
  TTree tout("t", "track and inscribed cylinder information");
  
  double xRpc, yRpc, zRpc;
  
  double theta, phi;
  
  int trackType;
  
  double xIavIn, yIavIn, zIavIn;
  double xIavOut, yIavOut, zIavOut;
  double trackLength, originSeparation;
  
  double xOSepaIavCoord, yOSepaIavCoord;
  
  bool twoContour, inscribable;
  
  double zInsCylTopTrackCoord, zInsCylBotTrackCoord;
  
  double xInsCylTopIavCoord, xInsCylBotIavCoord;
  double yInsCylTopIavCoord, yInsCylBotIavCoord;
  double zInsCylTopIavCoord, zInsCylBotIavCoord;
  
  /// tree branch assignment
  tout.Branch("xRpc", &xRpc, "xRpc/D");
  tout.Branch("yRpc", &yRpc, "yRpc/D");
  tout.Branch("zRpc", &zRpc, "zRpc/D");
  tout.Branch("theta", &theta, "theta/D");
  tout.Branch("phi", &phi, "phi/D");
  tout.Branch("trackType", &trackType, "trackType/I");
  tout.Branch("trackLength", &trackLength, "trackLength/D");
  tout.Branch("xIavIn", &xIavIn, "xIavIn/D");
  tout.Branch("yIavIn", &yIavIn, "yIavIn/D");
  tout.Branch("zIavIn", &zIavIn, "zIavIn/D");
  tout.Branch("xIavOut", &xIavOut, "xIavOut/D");
  tout.Branch("yIavOut", &yIavOut, "yIavOut/D");
  tout.Branch("zIavOut", &zIavOut, "zIavOut/D");
  tout.Branch("originSeparation", &originSeparation, "originSeparation/D");
  tout.Branch("xOSepaIavCoord", &xOSepaIavCoord, "xOSepaIavCoord/D");
  tout.Branch("yOSepaIavCoord", &yOSepaIavCoord, "yOSepaIavCoord/D");
  tout.Branch("twoContour", &twoContour, "twoContour/O");
  tout.Branch("inscribable", &inscribable, "inscribable/O");
  tout.Branch("zInsCylTopTrackCoord", &zInsCylTopTrackCoord, "zInsCylTopTrackCoord/D");
  tout.Branch("zInsCylBotTrackCoord", &zInsCylBotTrackCoord, "zInsCylBotTrackCoord/D");
  tout.Branch("xInsCylTopIavCoord", &xInsCylTopIavCoord, "xInsCylTopIavCoord/D");
  tout.Branch("yInsCylTopIavCoord", &yInsCylTopIavCoord, "yInsCylTopIavCoord/D");
  tout.Branch("zInsCylTopIavCoord", &zInsCylTopIavCoord, "zInsCylTopIavCoord/D");
  tout.Branch("xInsCylBotIavCoord", &xInsCylBotIavCoord, "xInsCylBotIavCoord/D");
  tout.Branch("yInsCylBotIavCoord", &yInsCylBotIavCoord, "yInsCylBotIavCoord/D");
  tout.Branch("zInsCylBotIavCoord", &zInsCylBotIavCoord, "zInsCylBotIavCoord/D");

  /// a valid event is an event going through IAV
  bool validEvent = false;
  
  int totLoop = 0, nValidEvent = 0;

  while(nValidEvent < nEvt)
  {
    
    totLoop++;
    
    TrackInfo ti;
    
    loopEvent(ti, validEvent);
    
    if( validEvent ) {
      
      nValidEvent++;
      
      xRpc = (double)ti.pRpc.X();
      
      yRpc = (double)ti.pRpc.Y();
      
      zRpc = (double)ti.pRpc.Z();
      
      theta = (double)ti.dirVec.Theta();
      
      phi = (double)ti.dirVec.Phi();
      
      trackType = (int)ti.trackIntersectType;
      
      trackLength = ti.trackLength;
      
      xIavIn = ti.pIavIn.X();
      yIavIn = ti.pIavIn.Y();
      zIavIn = ti.pIavIn.Z();
      xIavOut = ti.pIavOut.X();
      yIavOut = ti.pIavOut.Y();
      zIavOut = ti.pIavOut.Z();
      
      originSeparation = ti.originSeparation;
      
      xOSepaIavCoord = ti.pXYPlane.X();
      
      yOSepaIavCoord = ti.pXYPlane.Y();
      
      twoContour = ti.twoContour;
      
      inscribable = ti.inscribable;
      
      zInsCylTopTrackCoord = ti.zInsCylTopTrackCoord;
      
      zInsCylBotTrackCoord = ti.zInsCylBotTrackCoord;
      
      xInsCylTopIavCoord = ti.pInsCylTopIavCoord.X();
      
      yInsCylTopIavCoord = ti.pInsCylTopIavCoord.Y();
      
      zInsCylTopIavCoord = ti.pInsCylTopIavCoord.Z();
      
      xInsCylBotIavCoord = ti.pInsCylBotIavCoord.X();
      
      yInsCylBotIavCoord = ti.pInsCylBotIavCoord.Y();
      
      zInsCylBotIavCoord = ti.pInsCylBotIavCoord.Z();
      
      tout.Fill();
      
      /// if there is an intersection curve, save it
      const unsigned int np = ti.intersectCurve.size();
      
      if(np) {

        double x[np], y[np];
      
        for(unsigned int i = 0; i < np; i++) {
          
          x[i] = ti.intersectCurve[i].X();
          
          y[i] = ti.intersectCurve[i].Y();
          
        }
        
        fout.cd("graph");
        
        TGraph g(np, x, y);
        
        g.SetName(Form("g%i", nValidEvent));
        
        g.SetTitle(Form("g%i", nValidEvent));
        
        g.SetMarkerStyle(7);
        
        g.SetMarkerColor(kBlue);
        
        g.GetXaxis()->SetTitle("#theta");
        
        g.GetYaxis()->SetTitle("z_{track}");
        
        g.Write();
      
      }
      
    }
  }
  
  cout << nValidEvent << " " << totLoop << endl;
  
  fout.Write();
  
  fout.Close();
  
  return 0;
}
