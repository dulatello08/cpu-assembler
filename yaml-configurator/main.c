#include "main.h"

int main(int argc, char *argv[])
{
	FILE *fh;
	yaml_parser_t parser;
	yaml_event_t event;
	int done = 0;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return 1;
	}

	fh = fopen(argv[1], "r");
	if (!fh) {
		perror("fopen");
		return 1;
	}

	yaml_parser_initialize(&parser);
	yaml_parser_set_input_file(&parser, fh);

	do {
		if (!yaml_parser_parse(&parser, &event)) {
			fprintf(stderr, "Parser error %d\n", parser.error);
			return 1;
		}

		switch(event.type) {
		case YAML_NO_EVENT:
			break;
		case YAML_STREAM_START_EVENT:
			break;
		case YAML_STREAM_END_EVENT:
			break;
		case YAML_DOCUMENT_START_EVENT:
			break;
		case YAML_DOCUMENT_END_EVENT:
			break;
		case YAML_SEQUENCE_START_EVENT:
			break;
		case YAML_SEQUENCE_END_EVENT:
			break;
		case YAML_MAPPING_START_EVENT:
			break;
		case YAML_MAPPING_END_EVENT:
			break;
		case YAML_ALIAS_EVENT:
			break;
		case YAML_SCALAR_EVENT:
			printf("%s\n", event.data.scalar.value);
			break;
		}

		done = (event.type == YAML_STREAM_END_EVENT);
		yaml_event_delete(&event);
	} while (!done);

	yaml_parser_delete(&parser);
	fclose(fh);

	return 0;
}