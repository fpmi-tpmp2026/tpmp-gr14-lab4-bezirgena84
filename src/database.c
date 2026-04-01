#include "database.h"
#include <stdarg.h>

int db_init(sqlite3 **db, const char *filename) {
    int rc = sqlite3_open(filename, db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Ошибка открытия БД: %s\n", sqlite3_errmsg(*db));
        return rc;
    }
    
    // Включение поддержки внешних ключей
    sqlite3_exec(*db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);
    return SQLITE_OK;
}

void db_close(sqlite3 *db) {
    if (db) {
        sqlite3_close(db);
    }
}

int db_execute(sqlite3 *db, const char *sql) {
    char *errmsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL ошибка: %s\n", errmsg);
        sqlite3_free(errmsg);
    }
    return rc;
}

int db_query(sqlite3 *db, const char *sql,
             int (*callback)(void*, int, char**, char**),
             void *data) {
    char *errmsg = NULL;
    int rc = sqlite3_exec(db, sql, callback, data, &errmsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL ошибка: %s\n", errmsg);
        sqlite3_free(errmsg);
    }
    return rc;
}

int db_prepare_exec(sqlite3 *db, const char *sql, const char *types, ...) {
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки: %s\n", sqlite3_errmsg(db));
        return rc;
    }
    
    va_list args;
    va_start(args, types);
    
    for (int i = 0; types[i] != '\0'; i++) {
        switch (types[i]) {
            case 'i': // integer
                sqlite3_bind_int(stmt, i + 1, va_arg(args, int));
                break;
            case 'l': // long long
                sqlite3_bind_int64(stmt, i + 1, va_arg(args, long long));
                break;
            case 'd': // double
                sqlite3_bind_double(stmt, i + 1, va_arg(args, double));
                break;
            case 's': // string
                sqlite3_bind_text(stmt, i + 1, va_arg(args, char*), -1, SQLITE_STATIC);
                break;
            case 'b': // blob
                {
                    unsigned char *data = va_arg(args, unsigned char*);
                    int size = va_arg(args, int);
                    sqlite3_bind_blob(stmt, i + 1, data, size, SQLITE_STATIC);
                }
                break;
        }
    }
    va_end(args);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        fprintf(stderr, "Ошибка выполнения: %s\n", sqlite3_errmsg(db));
    }
    
    sqlite3_finalize(stmt);
    return rc;
}

long long db_last_insert_id(sqlite3 *db) {
    return sqlite3_last_insert_rowid(db);
}

int db_init_test_data(sqlite3 *db) {
    // Проверка наличия данных
    const char *check_sql = "SELECT COUNT(*) FROM country;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, check_sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    
    if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0) {
        sqlite3_finalize(stmt);
        return 0; // Данные уже есть
    }
    sqlite3_finalize(stmt);
    
    // Вставка тестовых стран
    const char *countries_sql[] = {
        "INSERT INTO country (name, capital, language, population_country, square_country, currency, head_country) VALUES ('Беларусь', 'Минск', 'Белорусский', 9400000, 207600.0, 'BYN', 'Президент');",
        "INSERT INTO country (name, capital, language, population_country, square_country, currency, head_country) VALUES ('Китай', 'Пекин', 'Китайский', 1440000000, 9596961.0, 'CNY', 'Председатель');",
        "INSERT INTO country (name, capital, language, population_country, square_country, currency, head_country) VALUES ('Россия', 'Москва', 'Русский', 146000000, 17098242.0, 'RUB', 'Президент');"
    };
    
    for (int i = 0; i < 3; i++) {
        if (db_execute(db, countries_sql[i]) != SQLITE_OK) {
            return -1;
        }
    }
    
    // Вставка регионов для Беларуси (id=1)
    const char *belarus_regions[] = {
        "INSERT INTO region (name, capital_region, population_region, square_region, country_id) VALUES ('Минская область', 'Минск', 1500000, 39900.0, 1);",
        "INSERT INTO region (name, capital_region, population_region, square_region, country_id) VALUES ('Брестская область', 'Брест', 1400000, 32800.0, 1);",
        "INSERT INTO region (name, capital_region, population_region, square_region, country_id) VALUES ('Витебская область', 'Витебск', 1200000, 40100.0, 1);"
    };
    
    // Вставка регионов для Китая (id=2)
    const char *china_regions[] = {
        "INSERT INTO region (name, capital_region, population_region, square_region, country_id) VALUES ('Гуандун', 'Гуанчжоу', 126000000, 179800.0, 2);",
        "INSERT INTO region (name, capital_region, population_region, square_region, country_id) VALUES ('Шаньдун', 'Цзинань', 101000000, 157100.0, 2);",
        "INSERT INTO region (name, capital_region, population_region, square_region, country_id) VALUES ('Сычуань', 'Чэнду', 83000000, 486000.0, 2);"
    };
    
    // Вставка регионов для России (id=3)
    const char *russia_regions[] = {
        "INSERT INTO region (name, capital_region, population_region, square_region, country_id) VALUES ('Московская область', 'Москва', 8500000, 44300.0, 3);",
        "INSERT INTO region (name, capital_region, population_region, square_region, country_id) VALUES ('Краснодарский край', 'Краснодар', 5600000, 75485.0, 3);",
        "INSERT INTO region (name, capital_region, population_region, square_region, country_id) VALUES ('Республика Крым', 'Симферополь', 1900000, 26081.0, 3);"
    };
    
    for (int i = 0; i < 3; i++) {
        db_execute(db, belarus_regions[i]);
        db_execute(db, china_regions[i]);
        db_execute(db, russia_regions[i]);
    }
    
    return 0;
}
