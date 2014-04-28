#ifndef _H_VSSEXTRACT
#define _H_VSSEXTRACT

#include <vector>
#include <string>
#include <Windows.h>
#include <iostream>
#include <fstream>

#include ".\include\sqlite3.h"
#pragma comment(lib, ".\\library\\sqlite.lib")


//#include "sqlite3.h"
//#pragma comment(lib, "sqlite3.lib")

#define DF_VSSEXTRACT_DBNAME "info.db"

typedef struct _stDbInfo{
   std::string FILENAME; 
   std::string FULLPATH;
   unsigned __int64 createTm, accessTm, mftTm;
}stDbInfo;

typedef std::vector<std::string> vcString;
typedef std::vector<stDbInfo> vcDbInfo;

class CVssExtract
{
private:
	std::string m_sTempFile;
	vcString m_vcVssPathList;
	vcDbInfo m_vcDbInfoList;
private:
	bool DropVssResult();
	bool IsUseOsVss();
	bool GetResultParser();
	bool GetDbInfoList(const char* a_sVssPath);
	void FindFiles(const char *path);
	bool GetVssTotalFileList();
	void ErrorLog(const char* a_sMes);
	void Log(const char* a_sMes);
	
public:
	CVssExtract();
	
public:
	bool GetVssList();
	bool CreateInfoDb(const char* a_sTableName);
};



#endif
