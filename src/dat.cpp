#include "dat.h"

#include "stdafx.h"


DatFile::DatFile(const char* filename)
{
	_numFiles = 0;

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

	_numFiles = _header.num_files;

	_files = new dat_offset[_numFiles];
	_file.seekg(_header.files, std::ios::beg);
	_file.read((char*)_files, sizeof(dat_offset) * _numFiles);

	_extensions = new dat_extension[_numFiles];
	_file.seekg(_header.extensions, std::ios::beg);
	_file.read((char*)_extensions, sizeof(dat_extension) * _numFiles);

	_names = new dat_name[_numFiles];
	_file.seekg(_header.names, std::ios::beg);
	uint32_t name_size;
	_file.read((char*)&name_size, sizeof(name_size));
	for (int i = 0; i < _numFiles; i++)
	{
		_file.read(_names[i], name_size);
	}

	_sizes = new dat_size[_numFiles];
	_file.seekg(_header.sizes, std::ios::beg);
	_file.read((char*)_sizes, sizeof(dat_size) * _numFiles);
}

DatFile::~DatFile()
{
	delete _files;
	delete _extensions;
	delete _names;
	delete _sizes;
}
