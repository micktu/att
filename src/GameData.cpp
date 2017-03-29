#include "GameData.h"
#include "utils.h"

#include <Pathcch.h>;


GameData::GameData(str_t &path)
{
	Read(path);
}

bool GameData::Read(str_t path)
{
	uint32_t attributes = GetFileAttributes(path.c_str());
	if (INVALID_FILE_ATTRIBUTES == attributes) return false;

	BasePath = path;

	DatFiles.clear();
	DatFiles.reserve(0x1000);
	GameFiles.clear();
	GameFiles.reserve(0x1000);

	if ((FILE_ATTRIBUTE_DIRECTORY & attributes) == 0)
	{
		if (!DatFile::CheckFile(path)) return false;
		DatFiles.emplace_back(new DatFile(path));
		return true;
	}

	str_vector_t files = find_files(path);

	path += L"\\";
	wchar_t pathTemp[MAX_PATH];
	for (str_t &filename : files)
	{
		if (!IsRelevantFile(filename)) continue;

		str_t fullPath = path + filename;
		GameFile* gf = new GameFile(filename, get_file_size(fullPath));
		GameFiles.emplace_back(gf);

		if (DatFile::CheckFile(fullPath))
		{
			DatFile *dat = new DatFile(fullPath);
			DatFiles.emplace_back(dat);

			filename.copy(pathTemp, MAX_PATH, 0);
			PathCchRemoveFileSpec(pathTemp, MAX_PATH);
			str_t relPath(pathTemp);

			gf->Dat = dat;
			gf->bIsContainer = true;
			gf->Files.reserve(dat->NumEntries());

			int i = 0;
			for (DatFileEntry &entry : *dat)
			{
				if (!IsRelevantFile(entry.Name)) continue;

				str_t relName = relPath + L"\\";
				relName += entry.Name;

				gf->Files.emplace_back(relName, entry.Size, gf, dat, i);
				++i;
			}

			continue;
		}
	}

	return true;
}

static const str_vector_t RELEVANT_EXTENSIONS = { L".dat", L".bin", L".tmd", L".smd", L".txt" };
bool GameData::IsRelevantFile(str_t &filename)
{
	return true;

	for (const str_t &ext : RELEVANT_EXTENSIONS)
	{
		if (0 == filename.compare(filename.length() - 4, filename.length(), ext))
		{
			return true;
		}
	}

	return false;
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
