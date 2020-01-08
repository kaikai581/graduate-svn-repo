/// usage:
/// [0] .L DrawDCSData.cpp+
/// [1] DrawDCSDataZoom(string RootFileName,
///                     string YAxisName,
///                     float YMax,
///                     float YMin)


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

void setGraphStyle(TGraph*, int, string);


void DrawDCSData(string infpn = "EH1TEMP.txt", float ymax = .99)
{
  ifstream inf(infpn.c_str());
  vector<string> fieldNames;
  char strtmp[MAX_CHAR];
  inf.getline(strtmp, MAX_CHAR);
  stringstream fieldLine(strtmp);
  while(fieldLine >> strtmp)
    fieldNames.push_back(strtmp);
  
  const int nsensors = fieldNames.size()-2;
  
  char s[MAX_CHAR];
  vector<int> id;
  vector<string> date_time;
  vector<float> sensor[nsensors];
  float maxVol = 0.;
  while(inf.getline(s, MAX_CHAR))
  {
    stringstream curLine;
    curLine << s;
    int curId;
    string curDate, curTime, curDateTime;
    float curReading;
    curLine >> curId;
    id.push_back(curId);
    curLine >> curDate;
    curLine >> curTime;
    curDateTime = curDate + " " + curTime;
    date_time.push_back(curDateTime);
    for(int i = 0; i < nsensors; i++)
    {
      curLine >> curReading;
      if(curReading > maxVol)
        maxVol = curReading;
      sensor[i].push_back(curReading);
    }
  }
  
  /// debug info
/*
  cout << id[id.size()-1] << endl;
  cout << date_time[date_time.size()-1] << endl;
  for(int i = 0; i < nsensors; i++)
    cout << sensor[i][sensor[i].size()-1] << " ";
  cout << endl;
*/

  TGraph *gSensor[nsensors];
  const unsigned int npoints = date_time.size();
  float x[npoints], y[nsensors][npoints];
  for(unsigned int i = 0; i < npoints; i++)
  {
    TDatime curDateTime(date_time[i].c_str());
    x[i] = curDateTime.Convert();
    for(int j = 0; j < nsensors; j++)
    {
      y[j][i] = sensor[j][i];
      float scaleFactor = 4000.;
      if(maxVol > 0.)
        scaleFactor = maxVol;
      y[j][i] *= ymax/scaleFactor;
    }
  }
  
  
  TLegend *lSensor = new TLegend(.11,.1,.31,.5);
  lSensor->SetFillColor(kWhite);
  lSensor->SetBorderSize(0);
  for(int i = 0; i < nsensors; i++)
  {
    gSensor[i] = new TGraph(npoints, x, y[i]);
    setGraphStyle(gSensor[i], i, fieldNames[i+2]);
    lSensor->AddEntry(gSensor[i], Form("%s", fieldNames[i+2].c_str()), "LP");
  }
  gSensor[0]->Draw("APL");
  lSensor->Draw();
  for(int i = 1; i < nsensors; i++)
    gSensor[i]->Draw("PL");
    
  TFile of("ENV.root", "recreate");
  for(int i = 0; i < nsensors; i++)
    gSensor[i]->Write();
  lSensor->Write();
  of.Close();
}


void setGraphStyle(TGraph* g, int idx, string ytitle)
{
  g->GetXaxis()->SetTimeDisplay(1);
  g->GetXaxis()->SetTimeFormat("%m/%d/%Y");
  g->GetXaxis()->SetNdivisions(-505);
  g->GetXaxis()->SetTimeOffset(0,"gmt");
  g->GetXaxis()->SetTitle("date (mm/dd/yyyy)");
  g->GetYaxis()->SetTitle(Form("%s (^{#circ}C)",  ytitle.c_str()));
  g->SetMarkerStyle(7);
  g->SetMarkerColor(idx%9+2);
  g->SetLineColor(idx%9+2);
  g->SetTitle("");
}



void DrawDCSDataZoom(string infpn = "EH1TEMP.txt",
     string ytitle = "temperature", float ymax = .99, float ymin = 0.)
{
  ifstream inf(infpn.c_str());
  vector<string> fieldNames;
  char strtmp[MAX_CHAR];
  inf.getline(strtmp, MAX_CHAR);
  stringstream fieldLine(strtmp);
  while(fieldLine >> strtmp)
    fieldNames.push_back(strtmp);
  
  const int nsensors = fieldNames.size()-2;
  
  char s[MAX_CHAR];
  vector<int> id;
  vector<string> date_time;
  vector<float> sensor[nsensors];
  float maxVol = 0.;
  while(inf.getline(s, MAX_CHAR))
  {
    stringstream curLine;
    curLine << s;
    int curId;
    string curDate, curTime, curDateTime;
    float curReading;
    curLine >> curId;
    id.push_back(curId);
    curLine >> curDate;
    curLine >> curTime;
    curDateTime = curDate + " " + curTime;
    date_time.push_back(curDateTime);
    for(int i = 0; i < nsensors; i++)
    {
      curLine >> curReading;
      if(curReading > maxVol)
        maxVol = curReading;
      sensor[i].push_back(curReading);
    }
  }

  TGraph *gSensor[nsensors];
  const unsigned int npoints = date_time.size();
  float x[npoints], y[nsensors][npoints];
  for(unsigned int i = 0; i < npoints; i++)
  {
    TDatime curDateTime(date_time[i].c_str());
    x[i] = curDateTime.Convert();
    for(int j = 0; j < nsensors; j++)
    {
      y[j][i] = sensor[j][i];
//      y[j][i] = (y[j][i]-3699.)/4.*(ymax-ymin)+ymin;
    }
  }
  
  
  TLegend *lSensor = new TLegend(.11,.1,.31,.5);
  lSensor->SetFillColor(kWhite);
  lSensor->SetBorderSize(0);
  for(int i = 0; i < nsensors; i++)
  {
    gSensor[i] = new TGraph(npoints, x, y[i]);
    gSensor[i]->SetName(fieldNames[i+2].c_str());
    gSensor[i]->SetTitle(fieldNames[i+2].c_str());
    //gSensor[i]->SetMaximum(ymax);
    //gSensor[i]->SetMinimum(ymin);
    setGraphStyle(gSensor[i], i, ytitle);
    lSensor->AddEntry(gSensor[i], Form("%s", fieldNames[i+2].c_str()), "LP");
  }
/*  gSensor[0]->Draw("APL");
  lSensor->Draw();
  for(int i = 1; i < nsensors; i++)
    gSensor[i]->Draw("PL");*/
    
  TFile of("ENV.root", "recreate");
  for(int i = 0; i < nsensors; i++)
    gSensor[i]->Write();
  lSensor->Write();
  of.Close();
}
