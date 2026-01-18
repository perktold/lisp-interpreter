#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include "../lisp_api.h"

value *hello_fun(env *e, value *args) {
	printf("Hello from dynamically loaded module!\n");
	return make_nil();
}

value *pg_execute(env *e, value *args) {
	value *fst_arg = eval(e, car(args));
	value *snd_arg = eval(e, car(cdr(args)));

	if (fst_arg->type != VT_STRING) {
		printf("not a string:");
		println_value(fst_arg);
		return make_nil(); //TODO: make error
	}
	if (snd_arg->type != VT_STRING) {
		printf("not a string:");
		println_value(snd_arg);
		return make_nil(); //TODO: make error
	}

	const char *conn_str = fst_arg->as.str;
	const char *query_str = snd_arg->as.str;

	PGconn *conn = PQconnectdb(conn_str);
	if (PQstatus(conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection failed: %s", PQerrorMessage(conn));
		PQfinish(conn);
		return make_nil(); //TODO: make error
	}

	PGresult *res = PQexec(conn, query_str);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "Query failed: %s", PQerrorMessage(conn));
		PQclear(res);
		PQfinish(conn);
		return make_nil(); //TODO: make error
	}

	int rows = PQntuples(res);
	int cols = PQnfields(res);

	value *table = make_nil();

	for (int row = 0; row < rows; row++) {
		value *row_val = make_nil();
		for (int col = 0; col < cols; col++) {
			switch (PQftype(res, col)) {
				case 23:
					row_val=cons(make_int(atoi(PQgetvalue(res, row, col))), row_val);
				break;
				case 1700:
				case 20:
					row_val=cons(make_int(atof(PQgetvalue(res, row, col))), row_val);
				break;
				default:
					const char *str;
					row_val=cons(make_string(PQgetvalue(res, row, col)), row_val);
				break;
			}
		}
		table = cons(reverse(row_val), table);
	}
	PQclear(res);
	PQfinish(conn);

	table = reverse(table);
	return table;
}

module_export *module_init() {
	static module_export exports[2];

	exports[0].name = "pg_execute";
	exports[0].v    = make_procedure(&pg_execute);

	exports[1].name = NULL;
	exports[1].v    = NULL;

	return exports;
}
