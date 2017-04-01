#pragma once

#include "stdafx.h"

#include "mruby.h"
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

#define STATE_IDLE 0
#define STATE_COLLECTING_STRINGS 1
#define STATE_EXPECTING_ARRAY 2
#define STATE_EXPECTING_ID 3

#define rstring_to_wstring(str) utf8_to_wstr(RSTRING_PTR(*str))
#define rstring_to_string(str) std::string(RSTRING_PTR(*str))
#define sym_to_wstring(mrb, irep, index) utf8_to_wstr(mrb_sym2name(mrb, irep->syms[index]))

struct ScriptMessage
{
	std::string Id;

	std::string Jp;
	std::string En;
	std::string Fr;
	std::string It;
	std::string De;
	std::string Sp;
	std::string Kr;
	std::string Cn;

	mrb_irep* irep;
	int Index;
};

struct ScriptContent
{
	std::vector<ScriptMessage> Messages;
	std::vector<std::vector<std::string>> Scenes;
};

struct subtitle_entry
{
	wchar_t id[0x44];
	wchar_t text[0x400];
};

struct subtitle_content
{
	uint32_t numEntries;
	subtitle_entry entries[];
};

struct subtitle_collection
{
	subtitle_content* jp;
	subtitle_content* en;
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

struct TranslationLine
{
	str_t Id;
	str_t Translation;
};

ScriptContent* script_extract(const char* bin);

void script_export(ScriptContent* content, str_t filename);

void script_dump_debug(const char* bin, str_t out_filename, ScriptContent* content = nullptr);

char* script_import(const char* bin, const char* filename, int* size);

ScriptMessage* script_find_messsage(ScriptContent *content, std::string id);
