#include "repl.h"
#include "constants.h"
#include "database.h"
#include "parser.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// META_COMMAND DEFINITION

#define EXIT_META ".exit"
#define LIST_META ".list"
#define USE_META ".use "
#define CREATE_DB ".create_db "
#define CREATE_TABLE ".create_table "
#define NEW_DATABASE ""

// END META_COMMAND DEFINITION

InputBuffer *new_input_buffer() {
    InputBuffer *input_buffer = (InputBuffer *)malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}

void print_prompt(database *selected_db) {
    if (selected_db == NULL) {
	printf("db > ");
	return;
    }
    printf("db [%s] > ", selected_db->name);
}

ReadResult read_input(InputBuffer *input_buffer) {
    ssize_t bytes_read =
	getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

    if (bytes_read <= 0) {
	if (errno == 0)
	    return READ_EOF;
	return READ_ERROR;
    }

    // Ignore trailing newline
    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
    return READ_SUCCESS;
}

void close_input_buffer(InputBuffer *input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}

MetaCommandResult do_meta_command(InputBuffer *input_buffer, app_state *state) {
    if (strcmp(input_buffer->buffer, EXIT_META) == 0) {
	//	close_input_buffer(input_buffer);
	return META_COMMAND_EXIT;
    } else if (strcmp(input_buffer->buffer, LIST_META) == 0) {
	if (state == NULL)
	    return META_COMMAND_SUCCESS;
	for (size_t i = 0; i < state->database_size; i++)
	    printf("[%s]\n", state->databases[i]->name);
	return META_COMMAND_SUCCESS;
    } else if (strncmp(input_buffer->buffer, USE_META, strlen(USE_META)) == 0) {
	char *tmp =
	    strdup(input_buffer->buffer + (sizeof(char) * strlen(USE_META)));
	remove_spaces(tmp, true);
	for (size_t i = 0; i < state->database_size; i++) {
	    if (strcmp(state->databases[i]->name, tmp) == 0) {
		state->selected_db = state->databases[i];
		free(tmp);
		return META_COMMAND_SUCCESS;
	    }
	}
	free(tmp);
	return META_COMMAND_FAILED;
    } else if (strncmp(input_buffer->buffer, CREATE_TABLE,
		       strlen(CREATE_TABLE)) == 0) {
	parse_table_definition(input_buffer->buffer +
			       (sizeof(char) * strlen(CREATE_TABLE)));
    }

    else

	return META_COMMAND_UNRECOGNIZED_COMMAND;
}

PrepareResult prepare_statement(InputBuffer *input_buffer,
				Statement *statement) {

    if (strncasecmp(input_buffer->buffer, INSERT_CLAUSE,
		    strlen(INSERT_CLAUSE)) == 0) {
	statement->type = STATEMENT_INSERT;
	return PREPARE_SUCCESS;
    }
    if (strncasecmp(input_buffer->buffer, SELECT_CLAUSE,
		    strlen(SELECT_CLAUSE)) == 0) {
	statement->type = STATEMENT_SELECT;
	return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void execute_statement(Statement *statement, app_state *state) {
    if (state->selected_db == NULL)
	return;
    switch (statement->type) {
    case (STATEMENT_INSERT):
	// TODO Implement the command here
	break;
    case (STATEMENT_SELECT):
	// TODO implement the command here
	break;
    }
}

void repl(void) {
    InputBuffer *input_buffer = new_input_buffer();
    app_state state;
    state.databases = NULL;
    state.selected_db = NULL;

    while (true) {
	print_prompt(state.selected_db);
	switch (read_input(input_buffer)) {
	case READ_SUCCESS:
	    break;
	case READ_ERROR:
	    printf("Error Reading input\n");
	    goto _exit;
	case READ_EOF:
	    printf("Goodbye\n");
	    goto _exit;
	}
	if (input_buffer->buffer[0] == '.') {
	    switch (do_meta_command(input_buffer, &state)) {
	    case (META_COMMAND_SUCCESS):
		continue;
	    case (META_COMMAND_UNRECOGNIZED_COMMAND):
		fprintf(stderr, "Unrecognized command '%s'\n",
			input_buffer->buffer);
		continue;
	    case (META_COMMAND_FAILED):
		fprintf(stderr, "Error with command '%s'\n",
			input_buffer->buffer);
		continue;
	    case (META_COMMAND_EXIT):
		goto _exit;
	    }
	}
	Statement statement;
	switch (prepare_statement(input_buffer, &statement)) {
	case (PREPARE_SUCCESS):
	    printf("recognized statement\n");
	    break;
	case (PREPARE_UNRECOGNIZED_STATEMENT):
	    fprintf(stderr, "Unrecognized keyword at start of '%s'.\n",
		    input_buffer->buffer);
	    continue;
	}
	execute_statement(&statement, &state);
	printf("Executed.\n");
    }
_exit:
    for (size_t i = 0; i < state.database_size; i++)
	free_database(state.databases[i]);
    free(state.databases);
}
