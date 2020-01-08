/********************************************************************\

  Name:         TrackLength.h
  Created by:   Jimmy Ngai
  
  Date:         April 2, 2013

  Contents:     Utility functions to calculate track length.

\********************************************************************/

#ifndef TRACKLENGTH_H
#define TRACKLENGTH_H

Double_t TrackLengthInCylinder(Double_t targetH, Double_t targetD, Double_t vx, Double_t vy, Double_t vz, Double_t dx, Double_t dy, Double_t dz)
{
    Double_t slopeX = dx / dz;
    Double_t slopeY = dy / dz;
    Double_t interceptX = vx - slopeX * vz;
    Double_t interceptY = vy - slopeY * vz;

    Double_t zBtm = -targetH / 2.0;
    Double_t xBtm = slopeX * zBtm + interceptX;
    Double_t yBtm = slopeY * zBtm + interceptY;
    Double_t zTop = targetH / 2.0;
    Double_t xTop = slopeX * zTop + interceptX;
    Double_t yTop = slopeY * zTop + interceptY;
    Double_t dX = xTop - xBtm;
    Double_t dY = yTop - yBtm;
    Double_t dZ = zTop - zBtm;
    Double_t radius = targetD / 2.0;

    Double_t xBtm2 = xBtm * xBtm;
    Double_t yBtm2 = yBtm * yBtm;
    Double_t xTop2 = xTop * xTop;
    Double_t yTop2 = yTop * yTop;
    Double_t dX2 = dX * dX;
    Double_t dY2 = dY * dY;
    Double_t dZ2 = dZ * dZ;
    Double_t radius2 = radius * radius;

    Double_t a0 = xBtm2 + yBtm2 - radius2;
    Double_t a1 = 2.0*(xBtm*dX + yBtm*dY);
    Double_t a2 = dX2 + dY2;
    Double_t discriminant = a1*a1 - 4.0*a0*a2;

    Double_t trackLength = 0.0;

    if (discriminant == 0.0) {
        if ((xBtm2 + yBtm2) <= radius2)
            trackLength = targetH;
    } else if (discriminant > 0.0) {
        Double_t discriminantRoot = TMath::Sqrt(discriminant);
        Double_t trackLength2DTotal = TMath::Sqrt(dX2 + dY2);
        Double_t trackLength2D = trackLength2DTotal;

        if ((xBtm2 + yBtm2) > radius2) {
            Double_t z = (-a1 - discriminantRoot) / (2.0 * a2);
            Double_t x = dX * z + xBtm;
            Double_t y = dY * z + yBtm;
            Double_t dXBtm = x - xBtm;
            Double_t dYBtm = y - yBtm;

            trackLength2D -= TMath::Sqrt(dXBtm*dXBtm + dYBtm*dYBtm);
        }
        if ((xTop2 + yTop2) > radius2) {
            Double_t z = (-a1 + discriminantRoot) / (2.0 * a2);
            Double_t x = dX * z + xBtm;
            Double_t y = dY * z + yBtm;
            Double_t dXTop = x - xTop;
            Double_t dYTop = y - yTop;

            trackLength2D -= TMath::Sqrt(dXTop*dXTop + dYTop*dYTop);
        }

        if (trackLength2D <= 0.0)
            trackLength = 0.0;
        else
            trackLength = TMath::Sqrt(dX2 + dY2 + dZ2) * trackLength2D / trackLength2DTotal;
    }

    return (trackLength);
}

Double_t TrackLengthInSphere(Double_t targetD, Double_t vx, Double_t vy, Double_t vz, Double_t dx, Double_t dy, Double_t dz)
{
    Double_t lengthV2 = vx*vx + vy*vy + vz*vz;
    Double_t dotVU = vx*dx + vy*dy + vz*dz;
    Double_t distance2 = lengthV2 - dotVU*dotVU;
    Double_t discriminant = targetD*targetD - 4.0*distance2;

    Double_t trackLength = 0.0;

    if (discriminant > 0.0)
        trackLength = TMath::Sqrt(discriminant);

    return (trackLength);
}

#endif
