#include "DatFile.h"
#include "utils.h"


bool DatFile::CheckFile(str_t &path)
{
	uint32_t attributes = GetFileAttributes(path.c_str());

	if (INVALID_FILE_ATTRIBUTES == attributes) return false;
	if (FILE_ATTRIBUTE_DIRECTORY & attributes) return false;

	uint32_t magic;
	std::ifstream file(path, std::ios::binary);
	file.read((char*)&magic, 4);
	file.close();

	return DAT_MAGIC == magic;
}

DatFile::DatFile(str_t &filename)
{
	DatFile();

	Read(filename);
}

bool DatFile::Read(str_t &filename)
{
	_filename = filename;
	_entries.clear();

	std::ifstream file(filename, std::ios::binary);

	if (!file.is_open())  return false;

	file.read((char*)&_header, sizeof(dat_header));
	if (DAT_MAGIC != _header.magic) return false;

	uint32_t numEntries = _header.num_files;

	if (numEntries < 1) return true;

	// Load file offsets
	std::vector<dat_offset_t> offsets(numEntries);
	file.seekg(_header.files);
	file.read((char*)&offsets[0], sizeof(dat_offset_t) * numEntries);

	// Load file sizes
	std::vector<dat_size_t> sizes(numEntries);
	file.seekg(_header.sizes);
	file.read((char*)&sizes[0], sizeof(dat_size_t) * numEntries);

	// Load file extensions
	std::vector<dat_ext_t> extensions(numEntries);
	file.seekg(_header.extensions);
	file.read((char*)&extensions[0], sizeof(dat_ext_t) * numEntries);

	// Load file names
	uint32_t name_size;
	std::vector<dat_name_t> names(numEntries);
	file.seekg(_header.names);
	file.read((char*)&name_size, sizeof(name_size));
	for (int i = 0; i < numEntries; i++)
	{
		file.read((char*)&names[i], name_size);
	}

	file.close();

	// Build entries
	_entries.reserve(numEntries);
	DatFileEntry entry;
	for (int i = 0; i < numEntries; i++)
	{
		entry.Index = i;
		entry.Offset = offsets[i];
		entry.Size = sizes[i];
		strcpy_s(entry.Extension, extensions[i]);
		entry.Name = cstr_to_wstr(names[i]);
		_entries.push_back(entry);
	}

	return true;
}

void DatFile::ReadFile(const DatFileEntry* entry, char* buffer)
{
	std::ifstream file(_filename, std::ios::binary);

	file.seekg(entry->Offset);
	file.read(buffer, entry->Size);
	file.close();
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

	file.close();
}

const DatFileEntry* DatFile::FindFile(str_t &name)
{
	for (DatFileEntry &entry : _entries)
	{
		if (name.compare(entry.Name) == 0)
		{
			return &entry;
		}
	}

	return nullptr;
}

void DatFile::ExtractFile(const DatFileEntry* entry, str_t outPath)
{
	char* buffer = new char[entry->Size];
	ReadFile(entry, buffer);

	outPath.reserve(MAX_PATH);
	outPath += L"\\";
	outPath += entry->Name;

	std::ofstream file(outPath, std::ios::binary);
	file.write(buffer, entry->Size);
	file.close();
}

void DatFile::ExtractAll(str_t &outPath)
{
	for (DatFileEntry &entry : _entries)
	{
		ExtractFile(&entry, outPath);
	}
}

