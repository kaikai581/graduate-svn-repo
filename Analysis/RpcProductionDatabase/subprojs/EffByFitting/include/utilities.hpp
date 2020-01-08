#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <string>


std::string trimTrailingSlash(std::string&);
bool checkInputValidity(std::string, int);
void loadDictionary();
bool isRootFile(std::string);
std::string getYear(std::string);
std::string getDate(std::string);
bool fExists(const char*);
void getModuleEffErr(double[], double[], double&, double&);
void eff2File(const char*, std::string, std::string, int, int, int, double,
              double, double, double, double[], double[]);


#endif
