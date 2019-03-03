#include <stdio.h>  
#include <string.h>    

#include "crc32.h"
#include <malloc.h>
#include <string>




int main(int argc, char** argv) {

	if (argc < 2) {
		return -1;
	}



	std::string fileName(argv[1]);

	init_CRC32_table();

	int nFile1 = 0;
	int nFile2 = 0;

	int nCrc1 = calcCrcCode2(fileName.c_str() , &nFile1);
	int nCrc2 = calcCrcCode(fileName.c_str() , &nFile2);


	printf("crc code %s , crc1 %d , crc2 %d , file1 %d , file2 %d\r\n", fileName.c_str(), nCrc1, nCrc2, nFile1, nFile2);
	
	return 0;
}

