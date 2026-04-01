#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <stdbool.h>

// Database connection
extern sqlite3* db;

// Function declarations
bool init_database(void);
void close_database(void);
bool execute_query(const char* sql);
int callback_select(void* data, int argc, char** argv, char** azColName);
bool table_exists(const char* table_name);

#endif