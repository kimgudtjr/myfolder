#include "CVssExtract.h"


CVssExtract::CVssExtract()
{
	

}

bool CVssExtract::DropVssResult()
{
	using std::cout;
	using std::endl;

	char sFileName[MAX_PATH] = {0};
	std::string sTempFile;
	std::string sCmd;

	tmpnam(sFileName);
	sTempFile = sFileName;
	sCmd = std::string("vssadmin list shadows | find \"ShadowCopy\" > ") +  &(sTempFile.c_str())[1];

	char sCurrentPath[MAX_PATH] = {0};

	if( GetCurrentDirectoryA(MAX_PATH,sCurrentPath) == NULL)
	{
		cout << "[ DropVssResult ] : False" << endl;
		cout << "**[ Error ]*** CVssExtract::DropVssResult in GetCurrentDirectoryA Error" << endl;
		return false;
	}

	this->m_sTempFile = sCurrentPath + sTempFile;
	//cout << sCmd << endl;
	system(sCmd.c_str());

	cout << "[ DropVssResult ] : OK" << endl;
	//cout << a_sTempFile << endl;
	return true;
}


bool CVssExtract::GetVssList()
{
	
	if(DropVssResult() == false)
		return false;

	if(IsUseOsVss() == false)
		return false;
	
	if(GetResultParser() == false)
		return false;

	if(GetVssTotalFileList() == false)
		return false;

	return true;
}


bool CVssExtract::IsUseOsVss()
{
	using std::cout;
	using std::endl;

	HANDLE hFile = NULL;

	hFile = CreateFileA(this->m_sTempFile.c_str(),GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		cout << "**[ Error ]*** CVssExtract::IsUseOsVss in CreateFileA Error" << endl;
		return false;
	}	
	DWORD dwSizeHigh = 0;
	DWORD dwSizeLow = 0;

	dwSizeLow = GetFileSize(hFile,&dwSizeHigh);	
	CloseHandle(hFile);

	if(dwSizeLow == 0){
		cout << "[ IsUseVss ] : False" << endl;
		return false;
	}
	else
	{
		cout << "[ IsUseVss ] : OK" << endl;
		return true;
	}
}

bool CVssExtract::GetResultParser()
{
	std::ifstream inFile;
	using std::cout;
	using std::endl;

	inFile.open(this->m_sTempFile);

	if(inFile.is_open() == false)
	{
		cout << "[ TempFile Read ] : False" << endl;
		inFile.close();
		return false;
	}
	
	cout << "[ TempFile Read ] : OK" << endl;

	char sBuf[MAX_PATH] = {0};
	std::string sTemp, sItem;
	int index = 0;
	
	const char *pStr = NULL;

	//pStr = &(s2.c_str()[2]);
	
	m_vcVssPathList.clear();
	
	while(inFile.getline(sBuf,MAX_PATH))
	{
		sTemp = sBuf;
		index = sTemp.find(':');
		sItem = sTemp.substr(index);
		sItem = &(sItem.c_str()[2]);
		//m_vcVssPathList.insert(sItem);
		m_vcVssPathList.push_back(sItem);
	}
	inFile.close();

	vcString::iterator it;
	
	cout << "[ Vss Name List ]" << endl;
	for(it = m_vcVssPathList.begin(); it != m_vcVssPathList.end(); it++)
	{
		cout << (*it) << endl;
	}
	
	return true;
}

bool CVssExtract::GetDbInfoList(const char* a_sVssPath)
{
	std::string sFindPath = std::string(a_sVssPath) + "\\*.*";
	
	m_vcDbInfoList.clear();
	FindFiles(sFindPath.c_str());

	char sFileName[_MAX_FNAME] = {0};
	_splitpath_s(a_sVssPath,NULL,NULL,NULL,NULL,sFileName,_MAX_FNAME,NULL,NULL);

	CreateInfoDb(sFileName);
	
	return true;
}

void CVssExtract::FindFiles(const char *path)
{
	HANDLE hSearch = NULL;
	WIN32_FIND_DATAA wfd = {0};
	BOOL bResult = TRUE;
	std::string sPath;
	char File[MAX_PATH] = {0};
	char Drive[_MAX_DRIVE] = {0};
	char Dir[MAX_PATH] = {0};

	hSearch = FindFirstFileA(path,&wfd);

	if(hSearch == INVALID_HANDLE_VALUE)	return;

	_splitpath_s(path,Drive,_MAX_DRIVE,Dir,MAX_PATH,NULL,NULL,NULL,NULL);

	while(bResult)
	{
		if( wfd.dwFileAttributes &	FILE_ATTRIBUTE_DIRECTORY )
		{
			if(strcmp(wfd.cFileName,".") && strcmp(wfd.cFileName,".."))
			{
				sPath = std::string(Drive) + Dir + wfd.cFileName + "\\*.*";
				FindFiles(sPath.c_str());
			}
		}
		else
		{
			stDbInfo Item;

			Item.FILENAME = wfd.cFileName;
			Item.FULLPATH = std::string(Drive) + Dir + wfd.cFileName;
			Item.createTm = (((unsigned __int64)wfd.ftCreationTime.dwHighDateTime) << 32) + wfd.ftCreationTime.dwLowDateTime;
			Item.accessTm = (((unsigned __int64)wfd.ftLastAccessTime.dwHighDateTime) << 32) + wfd.ftLastAccessTime.dwLowDateTime;
			Item.mftTm = (((unsigned __int64)wfd.ftLastWriteTime.dwHighDateTime) << 32) + wfd.ftLastWriteTime.dwLowDateTime;

			m_vcDbInfoList.push_back(Item);
		}

		bResult = FindNextFileA(hSearch,&wfd);
	}


	FindClose(hSearch);
}



bool CVssExtract::GetVssTotalFileList()
{
	if( m_vcVssPathList.size() == 0)
		return false;

	vcString::iterator it;

	//GetDbInfoList(m_vcVssPathList[0].c_str());

	for(it=m_vcVssPathList.begin(); it!=m_vcVssPathList.end(); it++)
	{
		GetDbInfoList((*it).c_str());
	}
	

	return true;
}


bool CVssExtract::CreateInfoDb(const char* a_sTableName)
{
	
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL; //sqlite3 statement 
	std::string sSql;


	if( sqlite3_open(DF_VSSEXTRACT_DBNAME,&db) == SQLITE_OK)
		Log("DB Open Ok");
	else
		ErrorLog("DB Open False");

	sSql = std::string("CREATE TABLE IF NOT EXISTS ") + a_sTableName + " (FILENAME TEXT, FULLPATH TEXT, createTm INT, accessTm INT, mftTm INT);";
	//sql = "CREATE TABLE IF NOT EXISTS MFT (FILENAME TEXT, FULLPATH TEXT, entry INT, ParentRef INT, Sl_writeTm INT, SI_createTm INT, SI_accessTm INT, SI_mftTm INT, FN_writeTm INT, FN_createTm INT, FN_accessTm INT, FN_mftTm INT, TYPE TEXT);";
	

	if( sqlite3_exec(db, sSql.c_str(), NULL, NULL, NULL) == SQLITE_OK) 
		Log("DB Table Create Ok");
    else 
        ErrorLog("DB Table Create False");

	sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

	sSql = std::string("INSERT INTO ") + a_sTableName + "(FILENAME,FULLPATH,createTm,accessTm,mftTm) VALUES (?1, ?2, ?3, ?4, ?5)";

	if(sqlite3_prepare_v2(db, sSql.c_str(), sSql.size(), &stmt, NULL) == SQLITE_OK)
		Log("sqlite3_prepare_v2 OK");
    else
		ErrorLog("sqlite3_prepare_v2 False");


	vcDbInfo::iterator it;
	stDbInfo Item;
	for(it = m_vcDbInfoList.begin(); it != m_vcDbInfoList.end(); it++)
	{
		Item = (*it);
		sqlite3_bind_text(stmt,1,Item.FILENAME.c_str(),Item.FILENAME.size(),SQLITE_STATIC);
		sqlite3_bind_text(stmt,2,Item.FULLPATH.c_str(),Item.FULLPATH.size(),SQLITE_STATIC);
		sqlite3_bind_int64(stmt,3,Item.createTm);
		sqlite3_bind_int64(stmt,4,Item.accessTm);
		sqlite3_bind_int64(stmt,5,Item.mftTm);

        if ( sqlite3_step(stmt) != SQLITE_DONE )
            ErrorLog("CreateInfoDb - sqlite3_step False");
        
		sqlite3_reset(stmt);
	}
	sqlite3_exec(db, "COMMIT TRANSACTION;", NULL, NULL,NULL);
	sqlite3_finalize(stmt);

	sqlite3_close(db);

	return true;
}

void CVssExtract::ErrorLog(const char* a_sMes)
{
	using std::cout;
	using std::endl;

	cout << "[ ErrorLog ] : " << a_sMes << endl;

	exit(0);
}

void CVssExtract::Log(const char* a_sMes)
{
	using std::cout;
	using std::endl;

	cout << "[ Log ] : " << a_sMes << endl;
}