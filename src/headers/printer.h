#ifndef PRINTER_H
#define PRINTER_H

#include "query.h"
#include "table.h"

void print_table(db_table *table, query_col **qcol, db_col_index **cols_index);

#endif
