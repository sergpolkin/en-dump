//---------------------------------------------------------------------------
//
//	Copyright (c) 1995, 1996. Windward Studios, Inc.
//	All Rights Reserved.
//
//---------------------------------------------------------------------------

#include "DataFile.h"

#include <list>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <numeric>

static char dataFileMagic[ 4 ] = { 'W','S','D','F' };
//----------------------------------------------------------------------
static std::string MakeLower ( const std::string & str ) {
	std::string dest;

	// Allocate the destination space
	dest.resize(str.size());

	// Convert the source string to lower case
	// storing the result in destination string
	std::transform(str.begin(), str.end(),
		dest.begin(), ::tolower);
	return dest;
}
//----------------------------------------------------------------------
bool DataFile::Init( const char *fileName ) {
	// if already open
	if(m_fileName.compare(fileName) == 0) return true;
	_Init (fileName);
	return true;
}
//----------------------------------------------------------------------
void DataFile::_Init( const char *fileName) {
	if ( !fileName ) throw("DataFile::_Init");

	m_fileName = fileName;

	if(m_fileStream.is_open()) m_fileStream.close();

	m_fileStream.open(m_fileName, std::ios::in | std::ios::binary);

	if ( !m_fileStream.is_open() ) throw("DataFile::_Init");

	m_fileStream.seekg (0, m_fileStream.end);
	uint32_t fileSize = m_fileStream.tellg();
	m_fileStream.seekg (0, m_fileStream.beg);

	//  The two entries in the file should be the magic number of the file and the
	//  size of the header in bytes.
	struct { char aMagicNum[ 4 ]; uint32_t tableSize; } dfHdr;

	m_fileStream.read( (char*)&dfHdr, sizeof(dfHdr) );

	if ( strncmp( dfHdr.aMagicNum, dataFileMagic, 4 ) != 0 ) {
		throw("DataFile::_Init");
	}

	//  Read in the entire file table before constructing the map.
	//  The input file is buffered, but this will still be faster.
	char *pFileTableBuff = new char[ dfHdr.tableSize ];

	m_fileStream.read( pFileTableBuff, dfHdr.tableSize );

	std::map<std::string, uint32_t> tmpMap;
	std::list<uint32_t> tmpList;

	char *pBuff = pFileTableBuff;
	while( pBuff < pFileTableBuff + dfHdr.tableSize ) {
		//  Get the string length
		uint32_t stringLength = *(uint32_t*)pBuff;
		pBuff += 4;

		//  Save a pointer to the string
		char *pStr = pBuff;
		pBuff += stringLength;

		//  Get the offset.
		uint32_t fileOffset = *(uint32_t*)pBuff;
		pBuff += 4;

		//  Add the string/offset pair to the map.
		std::string key = MakeLower(std::string(pStr));
		tmpMap[ key ] = fileOffset;

		tmpList.push_back(fileOffset); // Add offset to temp list
	}

	tmpList.sort();
	tmpList.push_back(fileSize);

	// Add to 'm_fileMap' pairs string/offset/size
	m_fileMap.clear();
	for(const auto & i : tmpMap) {
		const std::string& name = i.first;
		const uint32_t& offset = i.second;
		auto findIter = std::find(tmpList.begin(), tmpList.end(), offset);
		++findIter;
		FilePosition fp = { offset, (*findIter) - offset };
		m_fileMap[name] = fp;
	}

	//  Delete the file table buffer, which is no longer
	//  needed.
	delete[] pFileTableBuff;
}
//----------------------------------------------------------------------
void DataFile::Close()
{
	m_fileStream.close();
	m_fileName.clear();
	m_fileMap.clear();
}
//----------------------------------------------------------------------
void DataFile::Dump() {
	std::list<float> copy_time;
	for(const auto& i : m_fileMap) {
		const std::string& path = i.first;
		const FilePosition& filePos = i.second;
		auto fileName = path.substr(path.find_last_of("\\")+1);

		fprintf(stdout, "\t\t%s offset: %d size: %d",
			path.c_str(), filePos.offset, filePos.size);
		fflush(stdout);

		std::ofstream out;
		out.open(fileName, std::ios::out | std::ios::binary);
		if (!out.is_open()) throw("DataFile::Dump");

		auto start = std::chrono::high_resolution_clock::now();

		m_fileStream.seekg(filePos.offset, m_fileStream.beg);
		{
			const uint32_t bufSize = 1<<12;
			char buf[bufSize];
			uint32_t size = filePos.size;
			while (size > 0) {
				if (size > bufSize) {
					m_fileStream.read(buf, bufSize);
					size -= bufSize;
				}
				else {
					m_fileStream.read(buf, size);
					size = 0;
				}
				out.write(buf, m_fileStream.gcount());
			}
		}
		out.close();

		auto end = std::chrono::high_resolution_clock::now();

		typedef std::chrono::duration<float, std::milli> millisec_t;
		auto elapsed_msec = std::chrono::duration_cast<millisec_t>
			(end - start).count();
		fprintf(stdout, "\r%0.2f us\n", elapsed_msec);
		copy_time.push_back(elapsed_msec);
	}
	fprintf(stdout, "Total time: %0.2f\n",
		std::accumulate(copy_time.begin(), copy_time.end(), 0.f));
}
