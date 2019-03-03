#include "MergeTask.h"
#include <sstream>
#include "mergefile.h"
#include "utils.h"

int mergefile(const char** fileIn,
	int nFiles,
	const char* szOut);

int MergeTask::Run() {

	log_info("{%s:%s:%d} MergeTask  %s  Running @ %s ",
		__FILE__, __FUNCTION__, __LINE__, taskDescr().c_str(),
		getCurTime().c_str());

	int iFile = 0;
	std::string* sFiles = new std::string[m_nTotal];
	const char** sFileNames = (const char**)malloc(sizeof(char*) *  m_nTotal);
	if (NULL == sFiles )
		return -1;

	if (NULL == sFileNames) {
		delete[]sFiles;
		return -1;
	}
		
	std::string sOutFile = m_sPath + "/" + m_sFileName;

	for (iFile = 0; iFile < m_nTotal; iFile++) {
		std::stringstream ss;
		ss << m_sPath + "/" + m_sFileName;
		ss << "_t_" << m_nTotal;
		ss << "_i_" << iFile;
		sFiles[iFile] = ss.str();

		int nExist = file_exist(sFiles[iFile].c_str());

		if (!nExist) {
			delete[]sFiles;
			free(sFileNames);
			return -2;
		}

		sFileNames[iFile] = sFiles[iFile].c_str();

	}

	// if file exist delete file
	if (file_exist(sOutFile.c_str()))
	{
		remove_file((char*)sOutFile.c_str());
	}

	int nFiles = mergefile(sFileNames ,m_nTotal , sOutFile.c_str()) ;

	if (nFiles == m_nTotal) {
		
		log_info("{%s:%s:%d} MergeTask  %s  remove files @ %s",
			__FILE__, __FUNCTION__, __LINE__, taskDescr().c_str(),
			getCurTime().c_str());

		for (iFile = 0; iFile < m_nTotal; iFile++) {
			remove_file((char*)sFiles[iFile].c_str());
		}

	}

	delete[]sFiles;
	free(sFileNames);

	return nFiles == m_nTotal ? 0 : 1;
	
}