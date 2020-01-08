#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <set>
#include <vector>

using namespace std;


set<int> generateArray(unsigned int size)
{
  set<int> resarr;
  
  while(resarr.size() != size)
  {
    int element = rand() % 20 + 1;
    resarr.insert(element);
  }
  
  cout << "array: ";
  for(set<int>::iterator it = resarr.begin(); it!=resarr.end(); ++it)
    cout << *it << " ";
  cout << endl;
  
  return resarr;
}


int main()
{
  unsigned int arrsize1, arrsize2;
  set<int> arr1, arr2;
  
  srand(time(NULL));
  
  arrsize1 = rand() % 5 + 11;
  arrsize2 = rand() % 10 + 11;
  
  vector<int> resArr(max(arrsize1,arrsize2));
  
  arr1 = generateArray(arrsize1);
  arr2 = generateArray(arrsize2);
  
  vector<int>::iterator resit = set_intersection(arr1.begin(), arr1.end(), arr2.begin(), arr2.end(), resArr.begin());
  resArr.resize(resit-resArr.begin());
  
  cout << "intersection: ";
  for(unsigned int i = 0; i < resArr.size(); i++)
    cout << resArr[i] << " ";
  cout << endl;
  
  return 0;
}
