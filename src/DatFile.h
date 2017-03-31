#pragma once

#include "stdafx.h"


#define DAT_MAGIC '\x00TAD'
typedef uint32_t dat_offset_t, dat_size_t;
typedef char dat_name_t[MAX_PATH];
typedef char dat_ext_t[4];

struct dat_header
{
	uint32_t magic;
	uint32_t num_files;
	dat_offset_t files;
	dat_offset_t extensions;
	dat_offset_t names;
	dat_offset_t sizes;
	dat_offset_t unknown1;
	uint32_t unknown2;
};

struct DatFileEntry
{
	uint32_t Index;
	dat_ext_t Extension;
	str_t Name;
	dat_size_t Size;
	dat_offset_t Offset;
};

class DatFile
{
	str_t _filename;
	std::vector<DatFileEntry> _entries;
	dat_header _header;

public:

private:

public:
	static bool CheckFile(str_t &path);

	DatFile() = default;
	DatFile(const DatFile &obj) = default;
	DatFile(const str_t &filename);

	bool Read(const str_t &filename);
	void ReadFile(const DatFileEntry* entry, char* buffer);
	void InjectFile(int index, char * buffer, uint32_t numBytes);
	const DatFileEntry* FindFile(str_t &name);
	void ExtractFile(const DatFileEntry* entry, str_t outPath);
	void ExtractAll(str_t &outPath);

	FORCEINLINE str_t GetFilename() const { return _filename; }
	FORCEINLINE size_t NumEntries() const { return _entries.size(); }
	FORCEINLINE DatFileEntry* operator[](int index) { return &_entries[index]; }

	auto begin() { return _entries.begin(); }
	auto end() { return _entries.end(); }
};