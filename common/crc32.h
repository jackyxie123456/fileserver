#ifndef CRC_32__H_
#define CRC_32__H_

#ifdef __cplusplus
extern "C" {
#endif

	 void init_CRC32_table();

	 unsigned int crc32(unsigned char* buf, unsigned int len);

	 unsigned int crc32_len(unsigned int crc, unsigned char *buffer, unsigned int size);

	 unsigned int calcCrcCode(const char* fileName , int* nLen);

	 unsigned int  calcCrcCode2(const char* fileName,int* nLen);

#ifdef __cplusplus
}
#endif

#endif
