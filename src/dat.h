#pragma once

#include "stdafx.h"


typedef uint32_t dat_offset;

typedef uint32_t dat_size;

typedef struct dat_header
{
	uint32_t magic;
	uint32_t num_files;
	dat_offset files;
	dat_offset extensions;
	dat_offset names;
	dat_offset sizes;
	dat_offset unknown1;
	uint32_t unknown2;
} dat_header;

typedef union dat_extension
{
	char str[4];
	int num;
} dat_extension;

typedef char dat_name[256];

typedef struct dat_file
{
	FILE* file;
	dat_header header;
	int num_files;
	dat_offset* files;
	dat_extension* extensions;
	dat_name* names;
	dat_size* sizes;
} dat_file;

dat_file dat_open(const char* filename);

void dat_seek(dat_file dat, uint32_t index);

void dat_close(dat_file dat);
