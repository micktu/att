#include "GameData.h"

#include "utils.h"

GameData::GameData(str_t &path)
{
	GameData();
	Read(path);
}

bool GameData::Read(str_t path)
{
	uint32_t attributes = GetFileAttributes(path.c_str());
	if (INVALID_FILE_ATTRIBUTES == attributes) return false;

	BasePath = path;
	PlainFileNames.clear();
	DatFileNames.clear();
	DatFiles.clear();

	if (0 == (FILE_ATTRIBUTE_DIRECTORY & attributes))
	{
		if (!DatFile::CheckFile(path)) return false;
		DatFiles.emplace_back(path);
		return true;
	}

	str_vector_t files = find_files(path);

	path.append(L"\\");
	for (str_t &filename : files)
	{
		str_t fullPath = path + filename;
		if (DatFile::CheckFile(fullPath))
		{
			DatFileNames.push_back(filename);
			DatFiles.emplace_back(fullPath);
			continue;
		}

		PlainFileNames.push_back(filename);
	}

	return true;
}

void GameData::ListFiles()
{
	for (int i = 0; i < DatFiles.size(); ++i)
	{
		cout << DatFileNames[i] << '\n';

		DatFile& dat = DatFiles[i];

		for (int j = 0; j < dat.NumEntries(); ++j)
		{
			cout << L"-- " << dat[j]->Name << '\n';
		}
	}
}
