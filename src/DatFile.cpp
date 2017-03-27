#include "DatFile.h"

#include "stdafx.h"

DatFile::DatFile(const char* filename)
{
	_numEntries = 0;

	_file.open(filename, std::ios::in | std::ios::out | std::ios::binary);

	if (!_file.is_open())
	{
		std::cout << "Cannot open DAT file.";
		return;
	}
	
	_file.read((char*)&_header, sizeof(dat_header));

	if (_header.magic != '\x00TAD')
	{
		std::cout << "Malformed DAT header.";
		return;
	}

	_numEntries = _header.num_files;

	dat_offset* offsets = new dat_offset[_numEntries];
	_file.seekg(_header.files);
	_file.read((char*)offsets, sizeof(dat_offset) * _numEntries);

	dat_extension* extensions = new dat_extension[_numEntries];
	_file.seekg(_header.extensions);
	_file.read((char*)extensions, sizeof(dat_extension) * _numEntries);

	dat_name* names = new dat_name[_numEntries];
	_file.seekg(_header.names);
	uint32_t name_size;
	_file.read((char*)&name_size, sizeof(name_size));
	for (int i = 0; i < _numEntries; i++)
	{
		_file.read(names[i], name_size);
	}

	dat_size* sizes = new dat_size[_numEntries];
	_file.seekg(_header.sizes);
	_file.read((char*)sizes, sizeof(dat_size) * _numEntries);

	_entries = new DatFileEntry[_numEntries];

	DatFileEntry entry;

	for (int i = 0; i < _numEntries; i++)
	{
		strcpy_s(entry.Name, names[i]);
		entry.Extension = extensions[i];
		entry.Size = sizes[i];
		entry.Offset = offsets[i];

		_entries[i] = entry;
	}

	delete offsets;
	delete extensions;
	delete names;
	delete sizes;
}

void DatFile::ReadFile(int index, char* buffer)
{
	_file.seekg(_entries[index].Offset);
	_file.read(buffer, _entries[index].Size);
}

void DatFile::InjectFile(int index, char * buffer, uint32_t numBytes)
{
	_file.seekp(0, std::ios::end);
	uint32_t offset = (uint32_t)_file.tellp();
	_file.write(buffer, numBytes);

	_file.seekp(_header.files + sizeof(dat_offset) * index);
	_file.write((char*)&offset, sizeof(dat_offset));

	_file.seekp(_header.sizes + sizeof(dat_size) * index);
	_file.write((char*)&numBytes, sizeof(dat_size));
}

int DatFile::FindFile(const char * name)
{
	for (int i = 0; i < _numEntries; i++)
	{
		if (strcmp(name, _entries[i].Name) == 0)
		{
			return i;
		}
	}

	return -1;
}

void DatFile::ExtractFile(int index, const char * outPath)
{
	DatFileEntry* entry = &_entries[index];

	char* outFilename = new char[MAX_PATH];
	sprintf(outFilename, "%s\\%s", outPath, entry->Name);

	char* buffer = new char[entry->Size];
	ReadFile(index, buffer);

	std::fstream file(outFilename, std::ios::out | std::ios::binary);
	file.write(buffer, entry->Size);
	delete outFilename;
}

void DatFile::ExtractAll(const char * outPath)
{
	for (int i = 0; i < _numEntries; i++)
	{
		ExtractFile(i, outPath);
	}
}

void DatFile::PrintFiles()
{
	for (int i = 0; i < _numEntries; i++)
	{
		puts(_entries[i].Name);
	}
}

DatFile::~DatFile()

{
	if (_numEntries > 0)
	{
		delete _entries;
	}
}
