#ifndef REPL_H
#define REPL_H
#include "database.h"
#include <stddef.h>
typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_FAILED,
    META_COMMAND_UNRECOGNIZED_COMMAND,
    META_COMMAND_EXIT
} MetaCommandResult;

typedef enum { PREPARE_SUCCESS, PREPARE_UNRECOGNIZED_STATEMENT } PrepareResult;
typedef enum { READ_SUCCESS, READ_ERROR, READ_EOF } ReadResult;

typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;

typedef struct {
    StatementType type;
} Statement;

typedef struct {
    char *buffer;
    size_t buffer_length;
    size_t input_length;
} InputBuffer;

typedef struct {
    database *selected_db;
    database **databases;
    size_t database_size;
    size_t database_capacity;
} app_state;

void repl(void);

#endif
