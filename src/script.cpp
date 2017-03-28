#include "script.h"

script_content* collect_dialogue(mrb_state* mrb, mrb_irep* irep)
{
	script_content* content = new script_content;
	content->num_messages = 0;
	content->num_scenes = 0;

	int message_index;
	mrb_value messages[8];

	int counter = 0;
	int state = STATE_IDLE;

	for (int i = 0; i < irep->ilen; i++)
	{
		mrb_code code = irep->iseq[i];
		int opcode = GET_OPCODE(code);

		switch (state)
		{
		case STATE_IDLE:
		case STATE_COLLECTING_STRINGS:
			if (opcode == OP_STRING) {
				int index = GETARG_Bx(code);
				if (counter < 1) message_index = index;
				
				messages[counter++] = irep->pool[index];
				state = STATE_COLLECTING_STRINGS;

				if (counter > 7) state = STATE_EXPECTING_ARRAY;
				continue;
			}

			//if (counter > 0)
			//{
			//	printf("Found %d strings, and then reset.\n", counter);
			//	for (int j = 0; j < counter; j++)
			//	{
			//		mrb_p(mrb, messages[j]);
			//	}
			//}
			state = STATE_IDLE;
			counter = 0;
			break;

		case STATE_EXPECTING_ARRAY:
			if (opcode == OP_ARRAY)
			{
				state = STATE_EXPECTING_ID;
				continue;
			}

			printf("Expected OP_ARRAY, got %d @ %d | ", opcode, i);
			counter = 0;
			state = STATE_IDLE;
			break;

		case STATE_EXPECTING_ID:
			if (opcode == OP_SETCONST)
			{
				script_message message;
				const char* message_id = mrb_sym2name(mrb, irep->syms[GETARG_Bx(code)]);
				strcpy_s(message.id, message_id);

				copy_rstring(messages[0], message.jp);
				copy_rstring(messages[1], message.en);
				copy_rstring(messages[2], message.fr);
				copy_rstring(messages[3], message.it);
				copy_rstring(messages[4], message.de);
				copy_rstring(messages[5], message.sp);
				copy_rstring(messages[6], message.kr);
				copy_rstring(messages[7], message.cn);

				message.irep = irep;
				message.index = message_index;
				content->messages[content->num_messages++] = message;

				state = STATE_IDLE;
				counter = 0;
				continue;
			}

			counter = 0;
			printf("Expected OP_SETCONST, got %d @ %d | ", opcode, i);
			state = STATE_IDLE;
			break;
		}
	}

	bool found_mess = false;
	script_scene scene;
	scene.num_ids = 0;
	scene.ids = (char(*)[255])malloc(sizeof(char[255]) * 100);

	for (int i = 0; i < irep->slen; i++)
	{
		const char* sym = mrb_sym2name(mrb, irep->syms[i]);

		if (!found_mess && strstr(sym, "mess"))
		{
			found_mess = true;
			continue;
		}

		if (found_mess && sym[0] == 'M')
		{
			strcpy_s(scene.ids[scene.num_ids++], sym);
		}
	}

	if (scene.num_ids > 0)
	{
		content->scenes[content->num_scenes++] = scene;
	}

	for (int i = 0; i < irep->rlen; i++)
	{
		script_content* section = collect_dialogue(mrb, irep->reps[i]);

		memcpy(content->messages + content->num_messages, section->messages, sizeof(script_message) * section->num_messages);
		content->num_messages += section->num_messages;

		for (int j = 0; j < section->num_scenes; j++)
		{
			script_scene* scene = &section->scenes[j];
			if (scene->num_ids < 1) continue;
			content->scenes[content->num_scenes++] = *scene;
		}

		delete section;
	}

	return content;
}

script_content* script_extract(const char* bin)
{
	mrb_state* mrb = mrb_open();
	mrb_irep* irep = mrb_read_irep(mrb, (uint8_t*)bin);

	script_content* content = collect_dialogue(mrb, irep);
	mrb_close(mrb);

	return content;
}

void script_export(script_content* content, const char* filename)
{
	if (content->num_messages < 1) return;

	FILE* out;
	fopen_s(&out, filename, "wb");

	if (out == nullptr)
	{
		printf("Cannot write file %s", filename);
		return;
	}

	for (int i = 0; i < content->num_scenes; i++)
	{
		script_scene* scene = &content->scenes[i];

		fprintf(out, "### Scene %d\r\n\r\n", i + 1);

		for (int j = 0; j < scene->num_ids; j++)
		{
			script_message* message = script_find_messsage(content, scene->ids[j]);

			if (message > 0)
			{
				fprintf(out, "ID: %s\r\nJP: %s\r\nEN: %s\r\nRU: \r\n\r\n", scene->ids[j], message->jp, message->en);
			}
			else
			{
				fprintf(out, "ID: %s\r\nMESSAGE NOT FOUND", scene->ids[j]);
			}
		}

		fputs("\r\n", out);
	}

	fclose(out);
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

	script_content* content = collect_dialogue(mrb, irep);
	
	script_message* message = script_find_messsage(content, "M0010_S0005_S0000_101_a2b");

	const char* val = mrb_str_to_cstr(mrb, message->irep->pool[message->index + 1]);
	printf("\n%s\n\n", val);

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

script_message* script_find_messsage(script_content* content, const char* id)
{
	for (int i = 0; i < content->num_messages; i++)
	{
		if (strcmp(content->messages[i].id, id) == 0)
		{
			return &content->messages[i];
		}
	}

	return 0;
}