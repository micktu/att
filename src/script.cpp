#include "script.h"
#include "utils.h"


ScriptContent* collect_dialogue(mrb_state* mrb, mrb_irep* irep, wstr_t & filename)
{
	ScriptContent* content = new ScriptContent();

	int messageIndex = 0;
	std::vector<const mrb_value*> messages;

	TextState state = Idle;

	for (int i = 0; i < irep->ilen; i++)
	{
		mrb_code code = irep->iseq[i];
		int opcode = GET_OPCODE(code);

		switch (state)
		{
		case Idle:
		case CollectingStrings:
			if (OP_STRING == opcode)
			{
				int index = GETARG_Bx(code);
				if (messages.empty()) messageIndex = index;

				messages.push_back(&irep->pool[index]);
				state = CollectingStrings;

				if (messages.size() > 5) state = ExpectingArray;
				continue;
			}

			state = Idle;
			messages.clear();
			break;

		case ExpectingArray:
			if (OP_ARRAY == opcode)
			{
				state = ExpectingID;
				continue;
			}
			else if (OP_STRING == opcode && messages.size() < 8)
			{
				messages.push_back(&irep->pool[GETARG_Bx(code)]);
				continue;
			}

			wprintf_s(L"%s | Expected OP_ARRAY, got %d @ %d\r\n", filename.c_str(), opcode, i);
			messages.clear();
			state = Idle;
			break;

		case ExpectingID:
			if (OP_SETCONST == opcode)
			{
				str_t id = mrb_sym2name(mrb, irep->syms[GETARG_Bx(code)]);

				LocMessage message;
				message.Index = messageIndex;
				message.irep = irep;

				message.Id = id;

				message.Jp = rstring_to_string(messages[0]);
				message.En = rstring_to_string(messages[1]);
				message.Fr = rstring_to_string(messages[2]);
				message.It = rstring_to_string(messages[3]);
				message.De = rstring_to_string(messages[4]);
				message.Sp = rstring_to_string(messages[5]);
				//message.Kr = rstring_to_string(messages[6]);
				//message.Cn = rstring_to_string(messages[7]);
				
				content->Messages.emplace(id, message);

				state = Idle;
				messages.clear();
				continue;
			}

			wprintf_s(L"%s | Expected OP_SETCONST, got %d @ %d\r\n", filename.c_str(), opcode, i);
			messages.clear();
			state = Idle;
			break;
		}
	}

	bool bFound = false;
	str_vec_t scene;

	for (int i = 0; i < irep->slen; i++)
	{
		const char* s = mrb_sym2name(mrb, irep->syms[i]);

		if (s == nullptr)
		{
			//wc << std::endl << L"WARNING: unnamed symbol @ " << i << L" ";
			continue;
		}

		std::string sym(s);
		delete s;

		if (!bFound && (sym.find("mess") != -1 || sym.find("podtalk") != -1 || sym.find("sele_ex") != -1))
		{
			bFound = true;
			continue;
		}

		if (bFound && sym[0] == 'M')
		{
			scene.push_back(sym);
		}
	}

	if (!scene.empty())
	{
		content->Scenes.push_back(scene);
	}

	for (int i = 0; i < irep->rlen; i++)
	{
		ScriptContent* section = collect_dialogue(mrb, irep->reps[i], filename);

		auto& mes = content->Messages;
		auto& smes = section->Messages;
		mes.insert(smes.begin(), smes.end());

		for (str_vec_t& s : section->Scenes)
		{
			if (s.empty()) continue;
			content->Scenes.push_back(s);
		}

		delete section;
	}

	return content;
}

ScriptContent* script_extract(const char* bin, wstr_t &filename)
{
	mrb_state* mrb = mrb_open();
	mrb_irep* irep = mrb_read_irep(mrb, (uint8_t*)bin);

	if (irep == nullptr)
	{
		return nullptr;
	}

	ScriptContent* content = collect_dialogue(mrb, irep, filename);
	mrb_close(mrb);

	return content;
}

void script_export(ScriptContent* content, wstr_t filename)
{
	if (content->Scenes.empty())
	{
		if (!content->Messages.empty())
		{
			wc << L"Warning: " << content->Messages.size() << L" messages, ";
			wc << content->Scenes.size() << L" scenes in " << filename;
		}

		return;
	}

	std::ofstream file(filename);

	if (!file.is_open())
	{
		wc << "Cannot write file " << filename;
		return;
	}

	for (int i = 0; i < content->Scenes.size(); i++)
	{
		str_vec_t &scene = content->Scenes[i];
		
		file << "-- Section " << i + 1 << std::endl << std::endl;

		for (std::string &id : scene)
		{
			if (content->Messages.count(id) > 0)
			{
				LocMessage &message = content->Messages[id];
				file << format_loc_message(message);
				file << std::endl;
			}
			else
			{
				std::cout << id << " not found." << std::endl;
			}
		}

		file << std::endl;
	}

	file.close();
}

void fprint_irep(mrb_state* mrb, mrb_irep* irep, std::ofstream &out)
{
	//out << "IREP " << irep->filename;
	out << "IREP";
	out << " ilen: " << irep->ilen;
	out << " nlocals: " << irep->nlocals;
	out << " nregs: " << irep->nregs; 
	out << " plen: " << irep->plen;
	out << " rlen: " << irep->rlen;
	out << std::endl;

	out << std::endl << "Symbols:" << std::endl;
	for (int i = 0; i < irep->slen; i++)
	{
		const char* sym = mrb_sym2name(mrb, irep->syms[i]);

		if (sym == nullptr)
		{
			out << i << " (unnamed)" << std::endl;
			continue;
		}

		out << i << " " << sym << std::endl;
	}

	out << std::endl << "Pool:" << std::endl;
	for (int i = 0; i < irep->plen; i++)
	{
		if (irep->pool[i].tt != MRB_TT_STRING) continue;

		const char* val = mrb_str_to_cstr(mrb, irep->pool[i]);
		out << i << " " << val << std::endl;
	}

	out << std::endl << "Code:" << std::endl;
	for (int i = 0; i < irep->ilen; i++)
	{
		mrb_code c = irep->iseq[i];

		out << i << " OP: " << GET_OPCODE(c);
		out << " A: " << GETARG_A(c);
		out << " B: " << GETARG_B(c);
		out << " C: " << GETARG_C(c);
		out << " Bx: " << GETARG_Bx(c);
		out << std::endl;
	}

	out << std::endl;

	for (int i = 0; i < irep->rlen; i++)
	{
		fprint_irep(mrb, irep->reps[i], out);
	}
}

void script_dump_debug(const char* bin, wstr_t out_filename, ScriptContent* content)
{
	mrb_state* mrb = mrb_open();
	mrb_irep* irep = mrb_read_irep(mrb, (uint8_t*)bin);
	
	if (irep == nullptr) return;

	std::ofstream file(out_filename, std::ios::binary);
	fprint_irep(mrb, irep, file);
	mrb_close(mrb);

	if (content != nullptr)
	{
		for (auto &pair : content->Messages)
		{
			LocMessage &message = pair.second;

			file << std::endl;
			file << "ID :" << message.Id << std::endl;
			file << "JP :" << message.Jp << std::endl;
			file << "EN :" << message.En << std::endl;
			file << "FR :" << message.Fr << std::endl;
			file << "IT :" << message.It << std::endl;
			file << "DE :" << message.De << std::endl;
			file << "SP :" << message.Sp << std::endl;
			//file << "KR :" << message.Kr << std::endl;
			//file << "CN :" << message.Cn << std::endl;
		}

		for (str_vec_t &scene : content->Scenes)
		{
			file << std::endl;

			for (std::string &id : scene)
			{
				file << id << std::endl;
			}
		}
	}

	file.close();
}

char_vector_t script_import(str_map_t &messages, const char* bin, wstr_t &filename)
{
	mrb_state* mrb = mrb_open();
	mrb_irep* irep = mrb_read_irep(mrb, (uint8_t*)bin);

	ScriptContent* content = collect_dialogue(mrb, irep, filename);

	for(auto &message : messages)
	{
		LocMessage &locMessage = content->Messages[message.first];
		locMessage.irep->pool[locMessage.Index + 1] = mrb_str_new_cstr(mrb, message.second.c_str());
	}

	size_t outSize;
	uint8_t *buffer;
	mrb_dump_irep(mrb, irep, 0, &buffer, &outSize);

	char *p = (char *)buffer;
	char_vector_t out(p, p + outSize);

	mrb_free(mrb, buffer);
	mrb_close(mrb);

	return out;
}
