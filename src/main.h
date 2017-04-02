#pragma once

#include "stdafx.h"

int wmain(int argc, wchar_t * argv[], wchar_t * envp[]);

void DoHelp();
void DoList(int &argc, wchar_t ** &argv);

void DoExtract(int & argc, wchar_t **& argv);

void DoExport(int & argc, wchar_t **& argv);

void DoImport(int & argc, wchar_t **& argv);

void ReadGameData(class GameData& gd, const wstr_t filter, const wstr_t verb = L"");

static const std::map<wstr_t, cmd_callback_t> commands {
	{ L"list", &DoList },
	{ L"extract", &DoExtract },
	{ L"export", &DoExport },
	{ L"import", &DoImport },
};
