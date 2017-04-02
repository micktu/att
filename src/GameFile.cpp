#include "GameFile.h"
#include "GameData.h"
#include "DatFile.h"
#include "utils.h"


GameFile::GameFile(GameData *gf, const wstr_t &filename, size_t index, size_t size, size_t datIndex, size_t indexInDat) : FullPath(filename), Index(index), Size(size), DatIndex(datIndex), IndexInDat(indexInDat), Data(gf)
{
	split_path(FullPath, Path, Filename, Extension);
}

DatFileEntry *GameFile::GetDatEntry() const
{
	return DatIndex != ~0 ? &Data->GetDatFiles()[DatIndex][IndexInDat] : nullptr;
}
