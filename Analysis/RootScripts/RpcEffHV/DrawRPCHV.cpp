#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "TAxis.h"
#include "TDatime.h"
#include "TFile.h"
#include "TGraph.h"
#include "TLegend.h"

#define MAX_CHAR 100000

using namespace std;

void setGraphStyle(TGraph*, int);


void DrawRPCHV(string infpn = "EH1RPCHV.txt", float ymax = .99)
{
  ifstream inf(infpn.c_str());
  vector<string> fieldNames;
  char strtmp[MAX_CHAR];
  inf.getline(strtmp, MAX_CHAR);
  stringstream fieldLine(strtmp);
  while(fieldLine >> strtmp)
    fieldNames.push_back(strtmp);
  
  const int nchannel = fieldNames.size()-2;
  
  char s[MAX_CHAR];
  vector<int> id;
  vector<string> date_time;
  vector<float> voltage[nchannel];
  float maxVol = 0.;
  while(inf.getline(s, MAX_CHAR))
  {
    stringstream curLine;
    curLine << s;
    int curId;
    string curDate, curTime, curDateTime;
    float curVol;
    curLine >> curId;
    id.push_back(curId);
    curLine >> curDate;
    curLine >> curTime;
    curDateTime = curDate + " " + curTime;
    date_time.push_back(curDateTime);
    for(int i = 0; i < nchannel; i++)
    {
      curLine >> curVol;
      if(curVol > maxVol)
        maxVol = curVol;
      voltage[i].push_back(curVol);
    }
  }
  
  /// debug info
/*
  cout << id[id.size()-1] << endl;
  cout << date_time[date_time.size()-1] << endl;
  for(int i = 0; i < nchannel; i++)
    cout << voltage[i][voltage[i].size()-1] << " ";
  cout << endl;
*/

  TGraph *gVol[nchannel];
  const unsigned int npoints = date_time.size();
  float x[npoints], y[nchannel][npoints];
  for(unsigned int i = 0; i < npoints; i++)
  {
    TDatime curDateTime(date_time[i].c_str());
    x[i] = curDateTime.Convert();
    for(int j = 0; j < nchannel; j++)
    {
      y[j][i] = voltage[j][i];
      float scaleFactor = 4000.;
      if(maxVol > 0.)
        scaleFactor = maxVol;
      y[j][i] *= ymax/scaleFactor;
    }
  }
  
  
  TLegend *lVol = new TLegend(.11,.1,.31,.5);
  lVol->SetFillColor(kWhite);
  lVol->SetBorderSize(0);
  for(int i = 0; i < nchannel; i++)
  {
    gVol[i] = new TGraph(npoints, x, y[i]);
    setGraphStyle(gVol[i], i);
    lVol->AddEntry(gVol[i], Form("%s", fieldNames[i+2].c_str()), "LP");
  }
  gVol[0]->Draw("APL");
  lVol->Draw();
  for(int i = 1; i < nchannel; i++)
    gVol[i]->Draw("PL");
    
  TFile of("RPCVol.root", "recreate");
  for(int i = 0; i < nchannel; i++)
    gVol[i]->Write();
  lVol->Write();
  of.Close();
}


void setGraphStyle(TGraph* g, int idx)
{
  g->GetXaxis()->SetTimeDisplay(1);
  g->GetXaxis()->SetTimeFormat("%m/%d/%Y");
  g->GetXaxis()->SetNdivisions(-505);
  g->GetXaxis()->SetTimeOffset(0,"gmt");
  g->GetXaxis()->SetTitle("date (mm/dd/yyyy)");
  g->GetYaxis()->SetTitle("voltage (4kV)");
  g->SetMarkerStyle(7);
  g->SetMarkerColor(idx%9+2);
  g->SetLineColor(idx%9+2);
  g->SetTitle("");
}



void DrawRPCHVZoom(string infpn = "EH1RPCHV.txt", float ymax = .99, float ymin = 0.)
{
  ifstream inf(infpn.c_str());
  vector<string> fieldNames;
  char strtmp[MAX_CHAR];
  inf.getline(strtmp, MAX_CHAR);
  stringstream fieldLine(strtmp);
  while(fieldLine >> strtmp)
    fieldNames.push_back(strtmp);
  
  const int nchannel = fieldNames.size()-2;
  
  char s[MAX_CHAR];
  vector<int> id;
  vector<string> date_time;
  vector<float> voltage[nchannel];
  float maxVol = 0.;
  while(inf.getline(s, MAX_CHAR))
  {
    stringstream curLine;
    curLine << s;
    int curId;
    string curDate, curTime, curDateTime;
    float curVol;
    curLine >> curId;
    id.push_back(curId);
    curLine >> curDate;
    curLine >> curTime;
    curDateTime = curDate + " " + curTime;
    date_time.push_back(curDateTime);
    for(int i = 0; i < nchannel; i++)
    {
      curLine >> curVol;
      if(curVol > maxVol)
        maxVol = curVol;
      voltage[i].push_back(curVol);
    }
  }

  TGraph *gVol[nchannel];
  const unsigned int npoints = date_time.size();
  float x[npoints], y[nchannel][npoints];
  for(unsigned int i = 0; i < npoints; i++)
  {
    TDatime curDateTime(date_time[i].c_str());
    x[i] = curDateTime.Convert();
    for(int j = 0; j < nchannel; j++)
    {
      y[j][i] = voltage[j][i];
      y[j][i] = (y[j][i]-3699.)/4.*(ymax-ymin)+ymin;
    }
  }
  
  
  TLegend *lVol = new TLegend(.11,.1,.31,.5);
  lVol->SetFillColor(kWhite);
  lVol->SetBorderSize(0);
  for(int i = 0; i < nchannel; i++)
  {
    gVol[i] = new TGraph(npoints, x, y[i]);
    gVol[i]->SetMaximum(ymax);
    gVol[i]->SetMinimum(ymin);
    setGraphStyle(gVol[i], i);
    lVol->AddEntry(gVol[i], Form("%s", fieldNames[i+2].c_str()), "LP");
  }
/*  gVol[0]->Draw("APL");
  lVol->Draw();
  for(int i = 1; i < nchannel; i++)
    gVol[i]->Draw("PL");*/
    
  TFile of("RPCVol.root", "recreate");
  for(int i = 0; i < nchannel; i++)
    gVol[i]->Write();
  lVol->Write();
  of.Close();
}
