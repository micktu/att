#include "GameData.h"
#include "utils.h"


const ext_vector_t GameData::DAT_EXTENSIONS { L".dat", L".dtt", L".eff", L".evn" };
const ext_vector_t GameData::TEXT_EXTENSIONS { L".bin", L".tmd", L".smd", L".txt" };

GameData::GameData(str_t &path) : GameData()
{
	uint32_t attributes = GetFileAttributes(path.c_str());
	if (INVALID_FILE_ATTRIBUTES == attributes) return;
	if ((FILE_ATTRIBUTE_DIRECTORY & attributes) == 0) return;

	path = add_slash(path);
	Filenames = find_files(path);
	BasePath = path;
}

bool GameData::Read(str_t filter)
{
	DatFiles.clear();
	DatFiles.reserve(Filenames.size());
	GameFiles.clear();
	GameFiles.reserve(10 * Filenames.size());

	for (str_t& filename : Filenames)
	{
		ProcessFile(filename, filter);
	}

	return true;
}

void GameData::ProcessFile(const str_t &filename, const str_t &filter)
{
	str_t filePath = BasePath + filename;

	if (!IsDatFile(filePath) && IsRelevantFile(filename, filter))
	{
		GameFiles.emplace_back(filename, GameFiles.size(), get_file_size(filePath));
		return;
	}

	if (IsDatFile(filePath))
	{
		size_t datIndex = DatFiles.size();
		DatFiles.emplace_back(filePath);
		DatFile &df = DatFiles.back();

		str_t relPath = add_slash(filename);

		int i = 0;
		for (DatFileEntry &entry : df)
		{
			if (!IsRelevantFile(entry.Name, filter)) continue;
			GameFiles.emplace_back(relPath + entry.Name, GameFiles.size(), entry.Size, datIndex, i);
			++i;
		}

		return;
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
