#include "table.h"

void free_db_col_index(db_col_index **data) {
    if (data == NULL)
	return;
    for (size_t i = 0; data[i] != NULL)
	free(data[i]);
    free(data);
}

