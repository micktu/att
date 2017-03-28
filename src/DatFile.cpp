#include "DatFile.h"

bool DatFile::CheckFile(const wchar_t * path)
{
	uint32_t attributes = GetFileAttributes(path);

	if (INVALID_FILE_ATTRIBUTES == attributes) return false;
	if (FILE_ATTRIBUTE_DIRECTORY & attributes) return false;

	uint32_t magic;
	std::ifstream file(path, std::ios::binary);
	file.read((char*)&magic, 4);
	if (DAT_MAGIC != magic) return false;

	return true;
}

DatFile::DatFile(const wchar_t* filename)
{
	Read(filename);
}

bool DatFile::Read(const wchar_t* filename)
{
	wcscpy_s(_filename, filename);
	_entries.clear();

	std::ifstream file(filename, std::ios::binary);

	if (!file.is_open())  return false;

	file.read((char*)&_header, sizeof(dat_header));
	if (DAT_MAGIC != _header.magic) return false;

	uint32_t numEntries = _header.num_files;

	dat_offset_t* offsets = new dat_offset_t[numEntries];
	file.seekg(_header.files);
	file.read((char*)offsets, sizeof(dat_offset_t) * numEntries);

	dat_ext_t* extensions = new dat_ext_t[numEntries];
	file.seekg(_header.extensions);
	file.read((char*)extensions, sizeof(dat_ext_t) * numEntries);

	dat_name_t* names = new dat_name_t[numEntries];
	file.seekg(_header.names);
	uint32_t name_size;
	file.read((char*)&name_size, sizeof(name_size));
	for (int i = 0; i < numEntries; i++)
	{
		file.read(names[i], name_size);
	}

	dat_size_t* sizes = new dat_size_t[numEntries];
	file.seekg(_header.sizes);
	file.read((char*)sizes, sizeof(dat_size_t) * numEntries);

	DatFileEntry entry;
	//entry.Dat = this;

	for (int i = 0; i < numEntries; i++)
	{
		entry.Index = i;

		strcpy_s(entry.Name, names[i]);
		strcpy_s(entry.Extension, extensions[i]);
		entry.Size = sizes[i];
		entry.Offset = offsets[i];

		_entries.push_back(entry);
	}

	delete[] offsets;
	delete[] extensions;
	delete[] names;
	delete[] sizes;

	return true;
}

void DatFile::ReadFile(const DatFileEntry* entry, char* buffer)
{
	std::ifstream file(_filename, std::ios::binary);

	file.seekg(entry->Offset);
	file.read(buffer, entry->Size);
}

void DatFile::InjectFile(int index, char * buffer, uint32_t numBytes)
{
	std::ofstream file(_filename, std::ios::binary);

	file.seekp(0, std::ios::end);
	uint32_t offset = (uint32_t)file.tellp();
	file.write(buffer, numBytes);

	file.seekp(_header.files + sizeof(dat_offset_t) * index);
	file.write((char*)&offset, sizeof(dat_offset_t));

	file.seekp(_header.sizes + sizeof(dat_size_t) * index);
	file.write((char*)&numBytes, sizeof(dat_size_t));
}

const DatFileEntry* DatFile::FindFile(const char * name)
{
	for (std::vector<DatFileEntry>::iterator it = _entries.begin(); it != _entries.end(); ++it)
	{
		if (strcmp(name, it->Name) == 0)
		{
			return &(*it);
		}
	}

	return nullptr;
}

void DatFile::ExtractFile(const DatFileEntry* entry, const char * outPath)
{

	char* buffer = new char[entry->Size];
	ReadFile(entry, buffer);

	char* outFilename = new char[MAX_PATH];
	sprintf_s(outFilename, MAX_PATH, "%s\\%s", outPath, entry->Name);

	std::ofstream file(outFilename, std::ios::binary);
	file.write(buffer, entry->Size);
	delete[] outFilename;
}

void DatFile::ExtractAll(const char * outPath)
{
	for (std::vector<DatFileEntry>::iterator it = _entries.begin(); it != _entries.end(); ++it)
	{
		ExtractFile(&(*it), outPath);
	}
}

