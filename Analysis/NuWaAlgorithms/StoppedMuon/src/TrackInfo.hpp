#ifndef TRACKINFO_HPP
#define TRACKINFO_HPP

#include <vector>

#include "Math/Point2D.h"
#include "Math/Point3D.h"
#include "Math/Vector3D.h"


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
  
  /// track length
  double trackLength;

  /// point intersection type, if any
  point::IntersectType pointIntersectTypeIn, pointIntersectTypeOut;

  /// track intersection type, if any
  track::IntersectType trackIntersectType;
  
  /// data for the intersection curve of two infinite cylinders
  std::vector< ROOT::Math::XYPoint > intersectCurve;
  
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


#endif
