#include <cmath>
#include <iostream>
#include <vector>

using namespace std;

int onesComplement(int x)
{
  vector<int> binrep;
  while(x/2 > 0)
  {
    binrep.push_back(x%2);
    x /= 2;
  }
  binrep.push_back(x);
  
  cout << "binary representation of the input number is: ";
  for(int i = binrep.size()-1; i >= 0; i--)
    cout << binrep[i];
  cout << endl;
  
  cout << "binary representation of one's complement is: ";
  for(int i = binrep.size()-1; i >= 0; i--)
  {
    binrep[i] = !binrep[i];
    cout << binrep[i];
  }
  cout << endl;
  
  int res = 0;
  for(unsigned int i = 0; i < binrep.size(); i++)
  {
    res += binrep[i]*pow(2,i);
  }
  
  cout << "one's complement to " << x << " is ";
  cout << res << endl;
  
  return res;
}

int main()
{
  int x;
  
  cout << "input a positive integer: ";
  cin >> x;
  
  cout << "the number you input is " << x << endl;
  onesComplement(x);
}
