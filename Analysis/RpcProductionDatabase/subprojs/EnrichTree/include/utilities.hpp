#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <string>
#include <vector>


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
void getInputFilePathnames(std::vector<std::string>&, std::string);
std::string getRunNumber(std::string);
std::string getFileNumber(std::string);
std::string getOutputPathName(std::string, std::string);


#endif
