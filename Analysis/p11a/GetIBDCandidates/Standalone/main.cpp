#include <iostream>
#include "include/loopCoincidence.h"

using namespace std;

int main(int argc, char *argv[])
{
	string filename;
	if(argc == 2)
		filename = argv[1];
	loopCoincidence(filename);
	return 0;
}
