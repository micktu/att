#include "dat.h"

#include "stdafx.h"


dat_file dat_open(const char* filename)
{
	dat_file dat;
	dat.num_files = 0;

	FILE* file;
	fopen_s(&file, filename, "rb");
	dat.file = file;

	if (file == nullptr)
	{
		printf("Cannot open DAT file.");
		return dat;
	}

	fread(&dat.header, sizeof(dat_header), 1, file);

	if (dat.header.magic != '\x00TAD')
	{
		printf("Malformed DAT header.");
		return dat;
	}

	uint32_t num_files = dat.header.num_files;
	dat.num_files = num_files;

	uint32_t files_size = sizeof(dat_offset) * num_files;
	dat_offset* files = (dat_offset*)malloc(files_size);
	fseek(file, dat.header.files, SEEK_SET);
	fread(files, files_size, 1, file);
	dat.files = files;

	uint32_t extensions_size = sizeof(dat_extension) * num_files;
	dat_extension* extensions = (dat_extension*)malloc(extensions_size);
	fseek(file, dat.header.extensions, SEEK_SET);
	fread(extensions, extensions_size, 1, file);
	dat.extensions = extensions;

	uint32_t names_size = sizeof(dat_name) * num_files;
	uint32_t name_size;
	fseek(file, dat.header.names, SEEK_SET);
	fread(&name_size, sizeof(uint32_t), 1, file);
	dat_name* names = (dat_name*)malloc(names_size);

	for (uint32_t i = 0; i < num_files; i++)
	{
		fread(names[i], name_size, 1, file);
	}
	dat.names = names;

	uint32_t sizes_size = sizeof(dat_size) * num_files;
	dat_size* sizes = (dat_size*)malloc(sizes_size);
	fseek(file, dat.header.sizes, SEEK_SET);
	fread(sizes, sizes_size, 1, file);
	dat.sizes = sizes;

	return dat;
}

void dat_seek(dat_file dat, uint32_t index)
{
	fseek(dat.file, dat.files[index], SEEK_SET);
}

void dat_close(dat_file dat)
{
	fclose(dat.file);

	free(dat.files);
	free(dat.extensions);
	free(dat.names);
	free(dat.sizes);
}