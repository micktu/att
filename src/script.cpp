#include "script.h"

#include "stdafx.h"

#include "mruby.h"
#include "mruby/dump.h"
#include "mruby/opcode.h"
#include "mruby/string.h"


script_content* collect_dialogue(mrb_state* mrb, mrb_irep* irep)
{
	script_content* content = (script_content*)malloc(sizeof(script_content));
	content->num_messages = 0;
	content->num_scenes = 0;

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
				messages[counter++] = irep->pool[GETARG_Bx(code)];
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

			printf("Expected OP_ARRAY, got %d", opcode);
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

				content->messages[content->num_messages++] = message;

				state = STATE_IDLE;
				counter = 0;
				continue;
			}

			counter = 0;
			printf("Expected OP_SETCONST, got %d", opcode);
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

		free(section);
	}

	return content;
}

script_content* script_extract(FILE* script)
{
	mrb_state* mrb = mrb_open();
	mrb_irep* irep = mrb_read_irep_file(mrb, script);

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
		fprintf(out, "OP: %d A: %d B: %d: C: %d Bx: %d\n", GET_OPCODE(c), GETARG_A(c), GETARG_B(c), GETARG_C(c), GETARG_Bx(c));
	}

	fprintf(out, "\n");

	for (int i = 0; i < irep->rlen; i++)
	{
		fprint_irep(mrb, irep->reps[i], out);
	}
}

void script_export_debug(FILE* file, const char* out_filename)
{
	long initial_offset = ftell(file);

	mrb_state* mrb = mrb_open();
	mrb_irep* irep = mrb_read_irep_file(mrb, file);

	FILE* out;
	fopen_s(&out, out_filename, "wb");

	fprint_irep(mrb, irep, out);

	mrb_close(mrb);

	fseek(file, initial_offset, SEEK_SET);
	script_content* content = script_extract(file);

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

	fclose(out);
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