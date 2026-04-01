#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

// Инициализация подключения к БД
int db_init(sqlite3 **db, const char *filename);

// Закрытие подключения к БД
void db_close(sqlite3 *db);

// Выполнение запроса без возврата результатов
int db_execute(sqlite3 *db, const char *sql);

// Выполнение запроса с колбэком для обработки результатов
int db_query(sqlite3 *db, const char *sql, 
             int (*callback)(void*, int, char**, char**), 
             void *data);

// Подготовка и выполнение параметризованного запроса
int db_prepare_exec(sqlite3 *db, const char *sql, const char *types, ...);

// Получение последнего вставленного ID
long long db_last_insert_id(sqlite3 *db);

// Инициализация тестовых данных
int db_init_test_data(sqlite3 *db);

#endif // DATABASE_H
