#include "GameFile.h"
#include "GameData.h"
#include "DatFile.h"
#include "utils.h"


GameFile::GameFile(GameData *gf, const wstr_t &filename, size_t index, size_t size, size_t datIndex, size_t indexInDat) : Index(index), Size(size), DatIndex(datIndex), IndexInDat(indexInDat), Data(gf)
{
	split_path(filename, Path, Filename, Extension);
}

DatFileEntry *GameFile::GetResource() const
{
	return DatIndex != ~0 ? &Data->GetDatFiles()[DatIndex][IndexInDat] : nullptr;
}
