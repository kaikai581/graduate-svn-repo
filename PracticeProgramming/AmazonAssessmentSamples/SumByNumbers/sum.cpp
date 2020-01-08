#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;


vector<vector<int> > allcombination;


int decompose(int sum, vector<int>& inarr, unsigned int stage, vector<int>& quotients)
{
  //cout << "sum: " << sum << " " << "current element: " << inarr[stage] << endl;
  
  if(stage == inarr.size() - 1)
  {
    int totalnumbers = 0;
    if((sum%inarr[stage]) == 0)
    {
      quotients[stage] = sum/inarr[stage];
      for(unsigned int i = 0; i < quotients.size(); i++)
        totalnumbers += quotients[i];
      cout << "quotients: ";
      for(unsigned int i = 0; i < quotients.size(); i++)
        cout << quotients[i] << " ";
      cout << endl;
      allcombination.push_back(quotients);
    }
    
    if(totalnumbers) return 0;
  }
  else
  {
    int qmax = sum/inarr[stage];
    for(int i = 0; i <= qmax; i++)
    {
      quotients[stage] = i;
      decompose(sum-i*inarr[stage],inarr,stage+1,quotients);
      quotients[stage] = 0;
    }
    
  }
  
  if(allcombination.size())
  {
    int numele = 0;
    for(unsigned int i = 0; i < allcombination[0].size(); i++)
      numele += allcombination[0][i];
    return numele;
  }
  
  return -1;
}


int main()
{
  string instr, tmpstr;
  vector<int> inarr;
  int sum;
  
  cout << "please input an array of numbers separated by space: ";
  getline(cin, instr);
  cout << "please input a sum: ";
  cin >> sum;
  
  istringstream iss(instr);
  while(iss >> tmpstr)
    inarr.push_back(atoi(tmpstr.c_str()));
  
  cout << "input array of numbers: ";
  for(unsigned int i = 0; i < inarr.size(); i++)
    cout << inarr[i] << ((i == inarr.size()-1)? "":" ");
  cout << "; sum: " << sum << endl;
  
  vector<int> quotients(inarr.size());
  int res = decompose(sum, inarr, 0, quotients);

  //for(unsigned int i = 0; i < quotients.size(); i++)
    //cout << quotients[i] << " ";
  //cout << endl;
  if(res == -1) cout << "no solution" << endl;
  else cout << res << " numbers can make it" << endl;
}
