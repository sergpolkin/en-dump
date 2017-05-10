#include "DataFile.h"
#include <cstdio>
static void print_usage(const char* prog);
//----------------------------------------------------------------------
int main(int argc, char * argv[]) {
	if (argc < 2) {
		print_usage(argv[0]);
		return -1;
	}
	DataFile dataFile;
	dataFile.Init(argv[1]);
	dataFile.Dump();
	dataFile.Close();
	return 0;
}
//----------------------------------------------------------------------
void print_usage(const char* prog) {
	fprintf(stdout, "Usage: %s <enations.dat file>\n", prog);
}
