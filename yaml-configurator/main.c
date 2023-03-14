#include "main.h"

static void handle_event(cyaml_event_t event, cyaml_event_info_t *info, void *ctx)
{
	switch(event) {
	case CYAML_EVENT_ENTRY:
		printf("Entry\n");
		break;
	case CYAML_EVENT_EXIT:
		printf("Exit\n");
		break;
	case CYAML_EVENT_STRING:
		printf("String: %s\n", info->value.string);
		break;
	case CYAML_EVENT_INT:
		printf("Int: %d\n", info->value.integer);
		break;
	case CYAML_EVENT_MAP_START:
		printf("Map start\n");
		break;
	case CYAML_EVENT_MAP_END:
		printf("Map end\n");
		break;
	case CYAML_EVENT_SEQ_START:
		printf("Seq start\n");
		break;
	case CYAML_EVENT_SEQ_END:
		printf("Seq end\n");
		break;
	}
}

int main(int argc, char *argv[])
{
	cyaml_err_t err;
	cyaml_config_t config = {
		.log_level = CYAML_LOG_WARNING,
		.log_fn = cyaml_log,
	};
	cyaml_context_t ctx;
	foo_t foo;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <filename> <mode>\n", argv[0]);
		return 1;
	}

	ctx.config = &config;
	ctx.filename = argv[1];
	ctx.fh = fopen(argv[1], argv[2]);
	if (!ctx.fh) {
		perror("fopen");
		return 1;
	}

	ctx.state = CYAML_STATE_INIT_PARSE;

	err = cyaml_load_data(&ctx, &handle_event, &foo, foo_schema, NULL);
	if (err)
		fprintf(stderr, "Failed to parse file: %s\n", cyaml_strerror(err));

	return 0;
}