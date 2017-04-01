#pragma once

#include "stdafx.h"

int wmain(int argc, wchar_t * argv[], wchar_t * envp[]);

void DoHelp();
void DoList(int &argc, wchar_t ** &argv);

void DoExtract(int & argc, wchar_t **& argv);

void DoExport(int & argc, wchar_t **& argv);

void ReadGameData(class GameData& gd, const str_t filter, const str_t verb = L"");

static const std::map<str_t, cmd_t> commands {
	{ L"list", &DoList },
	{ L"extract", &DoExtract },
	{ L"export", &DoExport },
};
