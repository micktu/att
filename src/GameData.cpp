#include "GameData.h"
#include "utils.h"


const wcs_vector_t GameData::DAT_EXTENSIONS { L".dat", L".dtt", L".eff", L".evn" };
const wcs_vector_t GameData::TEXT_EXTENSIONS { L".bin", L".tmd", L".smd", L".txt" };

GameData::GameData(wstr_t &path) : GameData()
{
	uint32_t attributes = GetFileAttributes(path.c_str());
	if (INVALID_FILE_ATTRIBUTES == attributes) return;
	if ((FILE_ATTRIBUTE_DIRECTORY & attributes) == 0) return;

	path = add_slash(path);
	Filenames = find_files(path);
	BasePath = path;
}

bool GameData::Read(wstr_t filter)
{
	DatFiles.reserve(0x2B2B);
	DatFiles.clear();
	GameFiles.clear();

	for (wstr_t& filename : Filenames)
	{
		ProcessFile(filename, filter);
	}

	return true;
}

void GameData::ProcessFile(wstr_t &filename, const wstr_t &filter)
{
	wstr_t filePath = BasePath + filename;

	if (!IsDatFile(filePath) && IsRelevantFile(filename, filter))
	{
		GameFile gf(this, filename, GameFiles.size(), get_file_size(filePath));
		GameFiles.emplace(filePath, gf);
		return;
	}

	if (IsDatFile(filename))
	{
		size_t datIndex = DatFiles.size();
		DatFiles.emplace_back(BasePath, filename);
		DatFile &df = DatFiles.back();

		wstr_t relPath = add_slash(filename);

		int i = 0;
		for (DatFileEntry &entry : df)
		{
			if (IsRelevantFile(entry.Name, filter))
			{
				wstr_t fullName = relPath + entry.Name;
				GameFile gf(this, fullName, GameFiles.size(), entry.Size, datIndex, i);
				GameFiles.emplace(fullName, gf);
			}
			++i;
		}

		return;
	}
}

bool GameData::CheckExtension(const wstr_t &filename, const wcs_vector_t &list) const
{
	for (const wchar_t *ext : list)
	{
		if (ext_equals(filename, ext)) return true;
	}

	return false;
}

bool GameData::IsDatFile(const wstr_t &filename) const
{
	return CheckExtension(filename, DAT_EXTENSIONS);
}

bool GameData::IsTextFile(const wstr_t &filename) const
{
	return CheckExtension(filename, TEXT_EXTENSIONS);
}

bool GameData::IsRelevantFile(const wstr_t &filename, wstr_t filter) const
{
	if (filter.empty()) return true;

	std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

	if (filter.compare(L"data") == 0)
	{
		return IsDatFile(filename);
	}
	else if (filter.compare(L"text") == 0)
	{
		return IsTextFile(filename);
	}

	if (filter[0] != '.') filter = L"." + filter;
	return ext_equals(filename, filter.data());
}
