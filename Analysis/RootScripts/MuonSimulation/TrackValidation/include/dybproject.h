/********************************************************************\

  Name:         dybproject.h
  Created by:   Jimmy Ngai
  
  Date:         December 14, 2012

  Contents:     Daya Bay project common header.

\********************************************************************/

#ifndef DYBPROJECT_H
#define DYBPROJECT_H

// AD PMT number of rings.
const Int_t     kAdPmtNRing         = 8;
// AD PMT number of columns.
const Int_t     kAdPmtNColumn       = 24;
// AD PMT number of channels.
const Int_t     kAdPmtNChannel      = kAdPmtNRing * kAdPmtNColumn;

// AD dimensions (in mm).
const Double_t  kAdDiameter         =  5000.0;
const Double_t  kAdHeight           =  5000.0;
// AD 4-m AV dimensions (in mm).
const Double_t  kAdAv4Diameter      =  3964.0;
const Double_t  kAdAv4Height        =  3964.0;
// AD 3-m AV dimensions (in mm).
const Double_t  kAdAv3Diameter      =  3100.0;
const Double_t  kAdAv3Height        =  3070.0;

#endif
