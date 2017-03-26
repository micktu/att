// att.cpp : Defines the entry point for the console application.
//

#include "stdafx.h";

#include <windows.h>
#include <limits.h>

#include "dat.h"
#include "script.h"


void split_path_file(char* p, char* f, char *pf) {
	char *slash = pf, *next;
	while ((next = strpbrk(slash + 1, "\\/"))) slash = next;
	if (pf != slash) slash++;

	strncpy_s(p, slash - pf + 1, pf, slash - pf);
	strcpy_s(f, strlen(slash) + 1, slash);
}

int main(int argc, char* argv[])
{
	if (argc != 2) return 1;
	
	char* datFullPath = argv[1];

	DatFile dat(datFullPath);

	if (dat.NumFiles() == 0) return 1;

	char dat_path[MAX_PATH], dat_filename[MAX_PATH];
	split_path_file(dat_path, dat_filename, datFullPath);
	printf("DAT file %s is OK.\n\n ", dat_filename);

	char out_dir[MAX_PATH] = "out";
	CreateDirectoryA(out_dir, NULL);

	sprintf_s(out_dir, "%s\\%.*s", out_dir, (int)strlen(dat_filename) - 4, dat_filename);
	CreateDirectoryA(out_dir, NULL);

	sprintf_s(out_dir, "%s\\scripts", out_dir);
	CreateDirectoryA(out_dir, NULL);

	dat_extension* extensions = dat.GetExtensions();
	dat_name* names = dat.GetNames();

	for (int i = 0; i < dat.NumFiles(); i++)
	{
		if (extensions[i].num != '\x00nib') continue;

		dat_name &name = names[i];

		printf("Processing script %s... ", name);

		dat_offset* files = dat.GetFiles();

		FILE* tempFile;
		int err = fopen_s(&tempFile, datFullPath, "rb");
		fseek(tempFile, files[i], SEEK_SET);
		script_content* content = script_extract(tempFile);
		fclose(tempFile);

		char out_path[MAX_PATH];
		sprintf_s(out_path, "%s\\%.*s.txt", out_dir, (int)strlen(name) - 4, name);

		script_export(content, out_path);

		//strcpy(out_path + strlen(out_path) - 3, "debug.txt");
		//dat_seek(dat, i);
		//script_export_debug(dat.file, out_path);

		printf("Done.\n");
	}

	return 0;
}

