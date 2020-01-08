#include <string>

using namespace std;


string getYear(string ifStr)
{
  string year;
  for(int i = 0; i < 3; i++)
    ifStr = ifStr.substr(0, ifStr.rfind("/"));

  int lastSlashPosition = ifStr.rfind("/");

  if(lastSlashPosition > 4)
    year = ifStr.substr(lastSlashPosition-4, 4);
  return (year.length()) ? year : "";
}



string getDate(string ifStr)
{
  string date;
  int lastSlashPosition = ifStr.rfind("/");
  if(lastSlashPosition > 4)
    date = ifStr.substr(lastSlashPosition-4, 4);
  return (date.length()) ? date : "";
}

