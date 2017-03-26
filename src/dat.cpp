#include "dat.h"

#include "stdafx.h"


DatFile::DatFile(const char* filename)
{
	_numEntries = 0;

	_file.open(filename, std::ios::in | std::ios::binary);

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
		DatFileEntry entry;
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

DatFile::~DatFile()
{

}
