#include <fstream>
#include <iostream>
#include <stdlib.h>



void PlotEnAndRMS(string infname = "hall_en_binned_by_angle.txt")
{
  ifstream inf(infname.c_str());
  
  string tmp;
  int inpidx = 0;
  vector<int> vhall;
  vector<double> ven, vrms;
  while(inf >> tmp)
  {
    int col = inpidx % 3;
    int hall;
    double en, rms;
    if(col == 0)
    {
      hall = atoi(tmp.c_str());
    }
    else if(col == 1)
    {
      en = atof(tmp.c_str());
    }
    else if(col == 2)
    {
      rms = atof(tmp.c_str());
      if(en > 1.)
      {
        vhall.push_back(hall);
        ven.push_back(en);
        vrms.push_back(rms);
      }
    }
    inpidx++;
  }
  for(unsigned int i = 0; i < vhall.size(); i++)
  {
    cout << vhall[i] << " " << ven[i] << " " << vrms[i] << endl;
  }
  
  const int arrsize = vhall.size();
  int hall[arrsize];
  double en[arrsize], rms[arrsize];
  TGraphErrors* g = new TGraphErrors("")
}
