#include "script.h"
#include "utils.h"


ScriptContent* collect_dialogue(mrb_state* mrb, mrb_irep* irep)
{
	ScriptContent* content = new ScriptContent();

	int messageIndex;
	std::vector<mrb_value*> messages(8);

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

				if (messages.size() > 7) state = ExpectingArray;
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

			printf("Expected OP_ARRAY, got %d @ %d | ", opcode, i);
			messages.clear();
			state = Idle;
			break;

		case ExpectingID:
			if (OP_SETCONST == opcode)
			{
				ScriptMessage message;
				message.Id = mrb_sym2name(mrb, irep->syms[GETARG_Bx(code)]);
				message.Jp = rstring_to_string(messages[0]);
				message.En = rstring_to_string(messages[1]);
				message.Fr = rstring_to_string(messages[2]);
				message.It = rstring_to_string(messages[3]);
				message.De = rstring_to_string(messages[4]);
				message.Sp = rstring_to_string(messages[5]);
				message.Kr = rstring_to_string(messages[6]);
				message.Cn = rstring_to_string(messages[7]);
				message.Index = messageIndex;
				message.irep = irep;
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
	std::vector<std::string> scene;

	for (int i = 0; i < irep->slen; i++)
	{
		std::string sym = mrb_sym2name(mrb, irep->syms[i]);

		if (!bFound && sym.find("mess") != -1)
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

		for (std::vector<std::string>& scene : section->Scenes)
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

void script_export(ScriptContent* content, str_t filename)
{
	if (content->Messages.empty()) return;

	std::ofstream file(filename);

	if (!file.is_open())
	{
		wcout << "Cannot write file " << filename;
		return;
	}

	for (int i = 0; i < content->Scenes.size(); i++)
	{
		std::vector<std::string> &scene = content->Scenes[i];
		
		file << "### Scene " << i + 1 << std::endl << std::endl;

		for (std::string &id : scene)
		{
			const ScriptMessage* message = script_find_messsage(content, id);

			if (message != nullptr)
			{
				file << "ID: " << id << std::endl;
				file << "JP: " << message->Jp << std::endl;
				file << "EN: " << message->En << std::endl;
				file << "RU: " << "" << std::endl;
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

void fprint_irep(mrb_state* mrb, mrb_irep* irep, FILE* out)
{
	fprintf(out, "IREP %s ilen: %zd nlocals: %d nregs: %d plen: %zd rlen: %zd\n", irep->filename, irep->ilen, irep->nlocals, irep->nregs, irep->plen, irep->rlen);

	fprintf(out, "\nSymbols:\n");
	for (int i = 0; i < irep->slen; i++)
	{
		const char* sym = mrb_sym2name(mrb, irep->syms[i]);
		fprintf(out, "%d %s\n", i, sym);
	}

	fprintf(out, "\nPool:\n");
	for (int i = 0; i < irep->plen; i++)
	{
		if (irep->pool[i].tt != MRB_TT_STRING) continue;

		const char* val = mrb_str_to_cstr(mrb, irep->pool[i]);
		fprintf(out, "%d %s\n", i, val);
	}

	fprintf(out, "\nCode:\n");
	for (int i = 0; i < irep->ilen; i++)
	{
		mrb_code c = irep->iseq[i];
		fprintf(out, "%d OP: %d A: %d B: %d: C: %d Bx: %d\n", i, GET_OPCODE(c), GETARG_A(c), GETARG_B(c), GETARG_C(c), GETARG_Bx(c));
	}

	fprintf(out, "\n");

	for (int i = 0; i < irep->rlen; i++)
	{
		fprint_irep(mrb, irep->reps[i], out);
	}
}

void script_export_debug(const char* bin, const char* out_filename)
{
	mrb_state* mrb = mrb_open();
	mrb_irep* irep = mrb_read_irep(mrb, (uint8_t*)bin);

	FILE* out;
	fopen_s(&out, out_filename, "wb");

	fprint_irep(mrb, irep, out);

	mrb_close(mrb);

	/*
	script_content* content = script_extract(bin);

	for (int i = 0; i < content->num_messages; i++)
	{
		script_message* message = &content->messages[i];

		fputs("ID: ", out);
		fputs(message->id, out);
		fputs("\nJP: ", out);
		fputs(message->jp, out);
		fputs("\nEN: ", out);
		fputs(message->en, out);
		fputs("\nFR: ", out);
		fputs(message->fr, out);
		fputs("\nIT: ", out);
		fputs(message->it, out);
		fputs("\nDE: ", out);
		fputs(message->de, out);
		fputs("\nSP: ", out);
		fputs(message->sp, out);
		fputs("\nKR: ", out);
		fputs(message->kr, out);
		fputs("\nCN: ", out);
		fputs(message->cn, out);
		fputs("\n\n", out);
	}
	*/
	fclose(out);
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

ScriptMessage* script_find_messsage(ScriptContent *content, std::string id)
{
	for (ScriptMessage &message : content->Messages)
	{
		if (message.Id.compare(id) == 0)
		{
			return &message;
		}
	}

	return nullptr;
}