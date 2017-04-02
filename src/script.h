#pragma once

#include "stdafx.h"

#pragma warning(push)
#pragma warning(disable:4200)
#include "mruby.h"
#pragma warning(pop)

#include "mruby/dump.h"
#include "mruby/opcode.h"
#include "mruby/string.h"


enum TextState
{
	Idle,
	CollectingStrings,
	ExpectingArray,
	ExpectingID
};

#define rstring_to_wstring(str) utf8_to_wstr(RSTRING_PTR(*str))
#define rstring_to_string(str) std::string(RSTRING_PTR(*str))
#define sym_to_wstring(mrb, irep, index) utf8_to_wstr(mrb_sym2name(mrb, irep->syms[index]))

struct LocMessage
{
	str_t Id;

	str_t Jp;
	str_t En;
	str_t Fr;
	str_t It;
	str_t De;
	str_t Sp;
	str_t Kr;
	str_t Cn;

	str_t Ru;

	mrb_irep* irep;
	int Index;
};

using sub_content = std::map<std::wstring, LocMessage>;

struct ScriptContent
{
	std::vector<LocMessage> Messages;
	std::vector<str_vec_t> Scenes;
};

struct text_string
{
	uint32_t size;
	wchar_t* text;
};

struct text_entry
{
	text_string id;
	text_string value;
};

struct text_content
{
	uint32_t numEntries;
	text_entry* entries;
};


ScriptContent* script_extract(const char* bin, wstr_t &filename);

void script_export(ScriptContent* content, wstr_t filename);

void script_dump_debug(const char* bin, wstr_t out_filename, ScriptContent* content = nullptr);

char* script_import(const char* bin, const char* filename, size_t * size);

LocMessage* script_find_messsage(ScriptContent *content, std::string id);
