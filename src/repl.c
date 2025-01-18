#include "repl.h"
#include "constants.h"
#include "database.h"
#include "parser.h"
#include "save.h"
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
#define SAVE_META ".save"
#define LOAD_META ".load"

// END META_COMMAND DEFINITION

app_state *new_app_state() { return calloc(1, sizeof(app_state)); }

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

bool insert_table_to_state(app_state *state, db_table *table) {
    if (table == NULL)
	return false;
    db_table **tmp =
	realloc(state->selected_db->tables,
		sizeof(db_table *) * (state->selected_db->table_size + 1));
    if (tmp == NULL)
	return false;
    state->selected_db->tables = tmp;
    state->selected_db->tables[state->selected_db->table_size++] = table;
    return true;
}

bool insert_database_to_state(app_state *state, char *name) {
    database *db = new_database(name);
    if (db == NULL)
	return false;
    if (state->database_size >= state->database_capacity) {
	char *sdbname = NULL;
	if (state->selected_db != NULL) {
	    sdbname = strdup(state->selected_db->name);
	    if (sdbname == NULL) {
		free(db);
		return false;
	    }
	}

	void *tmp = realloc(state->databases, (state->database_capacity * 2) *
						  sizeof(database *));
	if (tmp == NULL) {
	    if (sdbname != NULL)
		free(sdbname);
	    free(db);
	    return false;
	}
	state->databases = tmp;
	state->database_capacity = state->database_capacity * 2;
	for (size_t i = 0; sdbname != NULL && i < state->database_size; i++) {
	    if (strcmp(sdbname, state->databases[i]->name) == 0)
		state->selected_db = state->databases[i];
	}
	if (sdbname != NULL)
	    free(sdbname);
    }
    state->databases[state->database_size++] = db;
    return true;
}

MetaCommandResult do_meta_command(InputBuffer *input_buffer, app_state *state) {
    if (strcmp(input_buffer->buffer, EXIT_META) == 0) {
	//	close_input_buffer(input_buffer);
	return META_COMMAND_EXIT;
    } else if (strcmp(input_buffer->buffer, LIST_META) == 0) {
	if (state == NULL)
	    return META_COMMAND_SUCCESS;
	for (size_t i = 0; i < state->database_size; i++) {
	    printf("YES\n");
	    for (size_t a = 0; a < state->databases[i]->table_size; a++) {
		for (size_t b = 0; b < state->databases[i]->tables[a]->col_size;
		     b++)
		    printf("[%s]:[%s]:[%s]\n", state->databases[i]->name,
			   state->databases[i]->tables[a]->name,
			   state->databases[i]->tables[a]->cols[b]->name);
	    }
	}
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
	if (state->selected_db == NULL) {
	    printf("Please select a database\n");
	    return META_COMMAND_FAILED;
	}
	db_table *table = parse_table_definition(
	    input_buffer->buffer + (sizeof(char) * strlen(CREATE_TABLE)));
	if (table == NULL)
	    return META_COMMAND_FAILED;
	return insert_table_to_state(state, table) ? META_COMMAND_SUCCESS
						   : META_COMMAND_FAILED;
    } else if (strncmp(input_buffer->buffer, CREATE_DB, strlen(CREATE_DB)) ==
	       0) {
	char *tmp = input_buffer->buffer + (sizeof(char) * strlen(CREATE_DB));
	if (!is_valid_column(tmp))
	    return META_COMMAND_FAILED;
	return insert_database_to_state(state, tmp) ? META_COMMAND_SUCCESS
						    : META_COMMAND_FAILED;
    } else if (strcmp(input_buffer->buffer, SAVE_META) == 0) {
	if (!save(state))
	    return META_COMMAND_FAILED;
	return META_COMMAND_SUCCESS;
    } else
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

void execute_statement(Statement *statement, app_state *state,
		       InputBuffer *input_buffer) {
    if (state->selected_db == NULL)
	return;
    switch (statement->type) {
    case (STATEMENT_INSERT):
	exec_insert_query(state->selected_db, input_buffer->buffer);

	// TODO Implement the command here
	break;
    case (STATEMENT_SELECT):
	exec_select_query(state->selected_db, input_buffer->buffer);
	// TODO implement the command here
	break;
    }
}

void repl(void) {
    InputBuffer *input_buffer = new_input_buffer();
    app_state *state = load();

    while (true) {
	print_prompt(state->selected_db);
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
	    switch (do_meta_command(input_buffer, state)) {
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
	execute_statement(&statement, state, input_buffer);
	printf("Executed.\n");
    }
_exit:
    for (size_t i = 0; i < state->database_size; i++)
	free_database(state->databases[i]);
    free(state->databases);
    free(state);
}
