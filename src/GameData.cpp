#include "GameData.h"
#include "utils.h"


const ext_vector_t GameData::DAT_EXTENSIONS { L".dat", L".dtt", L".eff", L".evn" };
const ext_vector_t GameData::TEXT_EXTENSIONS { L".bin", L".tmd", L".smd", L".txt" };

GameData::GameData(str_t &path)
{
	GameData();
	BasePath = path;
}

bool GameData::Read(str_t filter)
{
	uint32_t attributes = GetFileAttributes(BasePath.c_str());
	if (INVALID_FILE_ATTRIBUTES == attributes) return false;

	str_t path = BasePath;

	DatFiles.clear();
	GameFiles.clear();

	if ((FILE_ATTRIBUTE_DIRECTORY & attributes) == 0)
	{
		if (!DatFile::CheckFile(path)) return false;
		ProcessFile(path, path, filter);
		return true;
	}

	path = add_slash(path);
	str_vector_t files = find_files(path);
	GameFiles.reserve(files.size());
	DatFiles.reserve(files.size());
	for (str_t &filename : files)
	{
		ProcessFile(path, filename, filter);
	}

	return true;
}

void GameData::ProcessFile(const str_t &path, const str_t &filename, const str_t &filter)
{
	if (!IsRelevantFile(filename, filter)) return;

	str_t fullPath = path + filename;
	GameFile* gf(new GameFile(filename, get_file_size(fullPath)));
	GameFiles.emplace_back(gf);

	if (!IsDatFile(fullPath)) return;

	DatFile *dat = new DatFile(fullPath);
	DatFiles.emplace_back(dat);

	str_t relPath = path_strip_filename(filename);

	gf->Dat = dat;
	gf->bIsContainer = true;
	gf->Files.reserve(dat->NumEntries());

	int i = 0;
	for (DatFileEntry &entry : *dat)
	{
		if (!IsRelevantFile(entry.Name, filter)) continue;

		relPath = add_slash(relPath);
		gf->Files.emplace_back(relPath + entry.Name, entry.Size, gf, dat, i);
		++i;
	}
}

bool GameData::CheckExtension(const str_t &filename, const ext_vector_t &list) const
{
	for (wchar_t *ext : list)
	{
		if (ext_equals(filename, ext)) return true;
	}

	return false;
}

bool GameData::IsDatFile(const str_t &filename) const
{
	return CheckExtension(filename, DAT_EXTENSIONS);
}

bool GameData::IsTextFile(const str_t &filename) const
{
	return CheckExtension(filename, TEXT_EXTENSIONS);
}

bool GameData::IsRelevantFile(const str_t &filename, str_t filter) const
{
	if (filter.empty()) return true;

	std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);
	
	if (filter.compare(L"data") == 0)
	{
		return IsDatFile(filename);
	}
	if (filter.compare(L"text") == 0)
	{
		return IsTextFile(filename);
	}
	else
	{
		if (filter[0] != '.') filter = L"." + filter;
		return ext_equals(filename, filter.c_str());
	}
}

GameFile::GameFile(str_t filename, size_t size, const GameFile* container, const DatFile* dat, int index)
{
	RelPath = filename;
	split_path(RelPath, Filename, Extension);

	Size = size;
	Container = container;
	Dat = dat;
	Index = index;
}
