#include "script.h"
#include "utils.h"


ScriptContent* collect_dialogue(mrb_state* mrb, mrb_irep* irep)
{
	ScriptContent* content = new ScriptContent();

	int messageIndex;
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

			printf("Expected OP_ARRAY, got %d @ %d | ", opcode, i);
			messages.clear();
			state = Idle;
			break;

		case ExpectingID:
			if (OP_SETCONST == opcode)
			{
				LocMessage message;
				message.Index = messageIndex;
				message.irep = irep;

				message.Id = mrb_sym2name(mrb, irep->syms[GETARG_Bx(code)]);

				message.Jp = rstring_to_string(messages[0]);
				message.En = rstring_to_string(messages[1]);
				message.Fr = rstring_to_string(messages[2]);
				message.It = rstring_to_string(messages[3]);
				message.De = rstring_to_string(messages[4]);
				message.Sp = rstring_to_string(messages[5]);
				//message.Kr = rstring_to_string(messages[6]);
				//message.Cn = rstring_to_string(messages[7]);
				
				content->Messages.push_back(message);

				state = Idle;
				messages.clear();
				continue;
			}

			printf("Expected OP_SETCONST, got %d @ %d | ", opcode, i);
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
			wc << std::endl << L"WARNING: unnamed symbol @ " << i << L" ";
			continue;
		}

		std::string sym(s);

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
		ScriptContent* section = collect_dialogue(mrb, irep->reps[i]);

		auto& mes = content->Messages;
		auto& smes = section->Messages;
		mes.insert(mes.begin(), smes.begin(), smes.end());

		for (str_vec_t& scene : section->Scenes)
		{
			if (scene.empty()) continue;
			content->Scenes.push_back(scene);
		}

		delete section;
	}

	return content;
}

ScriptContent* script_extract(const char* bin)
{
	mrb_state* mrb = mrb_open();
	mrb_irep* irep = mrb_read_irep(mrb, (uint8_t*)bin);

	if (irep == nullptr)
	{
		return nullptr;
	}

	ScriptContent* content = collect_dialogue(mrb, irep);
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
		
		file << "### Scene " << i + 1 << std::endl << std::endl;

		for (std::string &id : scene)
		{
			LocMessage* message = script_find_messsage(content, id);

			if (message != nullptr)
			{
				file << format_loc_message(*message);
			}
			else
			{
				file << "ID: " << id << std::endl << "MESSAGE NOT FOUND" << std::endl;
			}

			file << std::endl;
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
		for (LocMessage &message : content->Messages)
		{
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

char* script_import(const char* bin, const char* filename, int* size)
{
	// Open text file
	// Find messages
	// Open irep

	mrb_state* mrb = mrb_open();
	mrb_irep* irep = mrb_read_irep(mrb, (uint8_t*)bin);

	ScriptContent* content = collect_dialogue(mrb, irep);

	//ScriptMessage* message = script_find_messsage(*content, "M0010_S0005_S0000_101_a2b");

	//const char* val = mrb_str_to_cstr(mrb, message->irep->pool[message->index + 1]);
	//printf("\n%s\n\n", val);

	//message->irep->pool[message->index + 1] = mrb_str_new_cstr(mrb, u8"Микту ебашит");

	size_t outSize;
	uint8_t* buffer = new uint8_t[1024 * 1024];
	mrb_dump_irep(mrb, irep, 0, &buffer, &outSize);

	mrb_close(mrb);

	char* outBuffer = new char[outSize];
	memcpy(outBuffer, buffer, outSize);

	*size = outSize;
	return outBuffer;

	// Replace messages in irep
	// dump irep
	// replace binary in dat

}

LocMessage* script_find_messsage(ScriptContent *content, std::string id)
{
	for (LocMessage &message : content->Messages)
	{
		if (message.Id.compare(id) == 0)
		{
			return &message;
		}
	}

	return nullptr;
}