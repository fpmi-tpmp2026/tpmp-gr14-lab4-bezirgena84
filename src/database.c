#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "../includes/database.h"

sqlite3* db = NULL;

bool init_database(void) {
    int rc = sqlite3_open("countries.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return false;
    }
    
    const char* create_users = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL,"
        "is_admin INTEGER DEFAULT 0"
        ");";
    
    const char* create_countries = 
        "CREATE TABLE IF NOT EXISTS countries ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT UNIQUE NOT NULL,"
        "capital TEXT NOT NULL,"
        "population INTEGER NOT NULL,"
        "flag_image TEXT"
        ");";
    
    const char* create_regions = 
        "CREATE TABLE IF NOT EXISTS regions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "country_id INTEGER NOT NULL,"
        "name TEXT NOT NULL,"
        "capital_city TEXT NOT NULL,"
        "population INTEGER NOT NULL,"
        "area REAL NOT NULL,"
        "map_image TEXT,"
        "FOREIGN KEY(country_id) REFERENCES countries(id) ON DELETE CASCADE,"
        "UNIQUE(country_id, name)"
        ");";
    
    char* err_msg = NULL;
    
    rc = sqlite3_exec(db, create_users, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    
    rc = sqlite3_exec(db, create_countries, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    
    rc = sqlite3_exec(db, create_regions, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    
    // Insert default admin user if not exists
    const char* check_admin = "SELECT COUNT(*) FROM users WHERE username='admin'";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, check_admin, -1, &stmt, 0);
    if (rc == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        if (count == 0) {
            // Password hash for "admin" (SHA-256)
            const char* insert_admin = 
                "INSERT INTO users (username, password, is_admin) VALUES "
                "('admin', '8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918', 1)";
            rc = sqlite3_exec(db, insert_admin, 0, 0, &err_msg);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "Failed to insert admin user: %s\n", err_msg);
                sqlite3_free(err_msg);
            }
        }
    }
    sqlite3_finalize(stmt);
    
    return true;
}

void close_database(void) {
    if (db) {
        sqlite3_close(db);
    }
}

bool execute_query(const char* sql) {
    char* err_msg = NULL;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}

int callback_select(void* data, int argc, char** argv, char** azColName) {
    (void)data; // Suppress unused parameter warning
    
    for(int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

bool table_exists(const char* table_name) {
    char sql[256];
    sprintf(sql, "SELECT name FROM sqlite_master WHERE type='table' AND name='%s'", table_name);
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return false;
    
    int exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}