#ifndef DATAFILE_H
#define DATAFILE_H

//---------------------------------------------------------------------------
//
//	Copyright (c) 1995, 1996. Windward Studios, Inc.
//	All Rights Reserved.
//
//---------------------------------------------------------------------------

#include <string>
#include <fstream>
#include <map>
#include <cstdint>

const int DEF_COUNTRY_CODE = 9;	// English

class DataFile
{
public:

	//  ctor/dtor do nothing.
	DataFile() {};
	~DataFile() {};

	bool Init(const char *fileName);

	// Dump to separate files
	void Dump();

protected:
	virtual void _Init( const char *fileName);

public:
	void Close();

protected:
	std::string m_fileName;
	std::ifstream m_fileStream;

	typedef struct {
		uint32_t offset;
		uint32_t size;
	} FilePosition;
	typedef std::map<std::string, FilePosition> MapFile;
	MapFile m_fileMap;
};

#endif
