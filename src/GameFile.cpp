#include "GameFile.h"
#include "utils.h"


GameFile::GameFile(str_t filename, size_t index, size_t size, size_t datIndex, size_t indexInDat) : Index(index), Size(size), DatIndex(datIndex), IndexInDat(indexInDat)
{
	split_path(filename, Path, Filename, Extension);
}