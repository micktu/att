#include "DatFile.h"

bool DatFile::CheckFile(const wchar_t * path)
{
	uint32_t attributes = GetFileAttributes(path);

	if (INVALID_FILE_ATTRIBUTES == attributes) return false;
	if (FILE_ATTRIBUTE_DIRECTORY & attributes) return false;

	uint32_t magic;
	FILE* file;
	_wfopen_s(&file, path, L"rb");
	fread_s(&magic, 4, 4, 1, file);
	fclose(file);
	if (DAT_MAGIC != magic) return false;

	return true;
}

DatFile::DatFile(const wchar_t* filename)
{
	Read(filename);
}

bool DatFile::Read(const wchar_t* filename)
{
	_numEntries = 0;

	std::fstream file(filename, std::ios::in | std::ios::binary);

	if (!file.is_open())  return false;

	file.read((char*)&_header, sizeof(dat_header));
	if (_header.magic != DAT_MAGIC) return false;

	_numEntries = _header.num_files;

	dat_offset_t* offsets = new dat_offset_t[_numEntries];
	file.seekg(_header.files);
	file.read((char*)offsets, sizeof(dat_offset_t) * _numEntries);

	dat_ext_t* extensions = new dat_ext_t[_numEntries];
	file.seekg(_header.extensions);
	file.read((char*)extensions, sizeof(dat_ext_t) * _numEntries);

	dat_name_t* names = new dat_name_t[_numEntries];
	file.seekg(_header.names);
	uint32_t name_size;
	file.read((char*)&name_size, sizeof(name_size));
	for (int i = 0; i < _numEntries; i++)
	{
		file.read(names[i], name_size);
	}

	dat_size_t* sizes = new dat_size_t[_numEntries];
	file.seekg(_header.sizes);
	file.read((char*)sizes, sizeof(dat_size_t) * _numEntries);

	_entries = new DatFileEntry[_numEntries];

	DatFileEntry entry;
	entry.Dat = this;

	for (int i = 0; i < _numEntries; i++)
	{
		entry.Index = i;

		strcpy_s(entry.Name, names[i]);
		strcpy_s(entry.Extension, extensions[i]);
		entry.Size = sizes[i];
		entry.Offset = offsets[i];

		_entries[i] = entry;
	}

	delete[] offsets;
	delete[] extensions;
	delete[] names;
	delete[] sizes;

	return true;
}

void DatFile::ReadFile(int index, char* buffer)
{
	std::fstream file(filename, std::ios::in | std::ios::binary);

	file.seekg(_entries[index].Offset);
	file.read(buffer, _entries[index].Size);
}

void DatFile::InjectFile(int index, char * buffer, uint32_t numBytes)
{
	std::fstream file(filename, std::ios::out | std::ios::binary);

	file.seekp(0, std::ios::end);
	uint32_t offset = (uint32_t)file.tellp();
	file.write(buffer, numBytes);

	file.seekp(_header.files + sizeof(dat_offset_t) * index);
	file.write((char*)&offset, sizeof(dat_offset_t));

	file.seekp(_header.sizes + sizeof(dat_size_t) * index);
	file.write((char*)&numBytes, sizeof(dat_size_t));
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
	sprintf_s(outFilename, MAX_PATH, "%s\\%s", outPath, entry->Name);

	char* buffer = new char[entry->Size];
	ReadFile(index, buffer);

	std::fstream file(outFilename, std::ios::out | std::ios::binary);
	file.write(buffer, entry->Size);
	delete[] outFilename;
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
