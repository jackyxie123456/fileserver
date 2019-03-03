#ifndef UTILS__2__H_
#define UTILS__2__H_

#include <iostream>

int local_file_list(const char *path, std::string* outfiles, int maxNo);
int local_file_list1(const char *path, std::string* outfiles, int nStart, int maxN);

#endif