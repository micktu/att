#pragma once

#include "stdafx.h"


class DatFile;

struct GameFile
{
	GameFile() = default;
	GameFile(str_t filename, size_t index, size_t size = -1, size_t datIndex = -1, size_t indexInDat = -1);

	str_t Path;
	str_t Filename;
	str_t Extension;

	size_t Index;
	size_t Size;
	size_t DatIndex;
	size_t IndexInDat;
};
