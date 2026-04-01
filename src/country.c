#include "country.h"
#include <string.h>

bool country_create(sqlite3 *db, Country *country) {
    const char *sql = "INSERT INTO country (name, capital, language, population_country, square_country, currency, head_country, flag_image) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, country->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, country->capital, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, country->language, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 4, country->population);
    sqlite3_bind_double(stmt, 5, country->square);
    sqlite3_bind_text(stmt, 6, country->currency, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, country->head, -1, SQLITE_STATIC);
    
    if (country->flag_image && country->flag_size > 0) {
        sqlite3_bind_blob(stmt, 8, country->flag_image, country->flag_size, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 8);
    }
    
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        country->id = sqlite3_last_insert_rowid(db);
    }
    
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool country_get_by_id(sqlite3 *db, int id, Country *country) {
    const char *sql = "SELECT id, name, capital, language, population_country, square_country, currency, head_country FROM country WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        country->id = sqlite3_column_int(stmt, 0);
        strncpy(country->name, (const char*)sqlite3_column_text(stmt, 1), 100);
        strncpy(country->capital, (const char*)sqlite3_column_text(stmt, 2), 100);
        strncpy(country->language, (const char*)sqlite3_column_text(stmt, 3), 50);
        country->population = sqlite3_column_int64(stmt, 4);
        country->square = sqlite3_column_double(stmt, 5);
        strncpy(country->currency, (const char*)sqlite3_column_text(stmt, 6), 3);
        strncpy(country->head, (const char*)sqlite3_column_text(stmt, 7), 100);
        found = true;
    }
    
    sqlite3_finalize(stmt);
    return found;
}

int country_get_all(sqlite3 *db, Country **countries) {
    const char *sql = "SELECT id, name, capital, language, population_country, square_country, currency, head_country FROM country;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    
    // Подсчет количества стран
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) count++;
    sqlite3_reset(stmt);
    
    if (count == 0) {
        sqlite3_finalize(stmt);
        *countries = NULL;
        return 0;
    }
    
    *countries = calloc(count, sizeof(Country));
    if (!*countries) {
        sqlite3_finalize(stmt);
        return -1;
    }
    
    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < count) {
        Country *c = &(*countries)[i++];
        c->id = sqlite3_column_int(stmt, 0);
        strncpy(c->name, (const char*)sqlite3_column_text(stmt, 1), 100);
        strncpy(c->capital, (const char*)sqlite3_column_text(stmt, 2), 100);
        strncpy(c->language, (const char*)sqlite3_column_text(stmt, 3), 50);
        c->population = sqlite3_column_int64(stmt, 4);
        c->square = sqlite3_column_double(stmt, 5);
        strncpy(c->currency, (const char*)sqlite3_column_text(stmt, 6), 3);
        strncpy(c->head, (const char*)sqlite3_column_text(stmt, 7), 100);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

bool country_update(sqlite3 *db, Country *country) {
    const char *sql = "UPDATE country SET name=?, capital=?, language=?, population_country=?, square_country=?, currency=?, head_country=? WHERE id=?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, country->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, country->capital, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, country->language, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 4, country->population);
    sqlite3_bind_double(stmt, 5, country->square);
    sqlite3_bind_text(stmt, 6, country->currency, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, country->head, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 8, country->id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool country_delete(sqlite3 *db, int id) {
    // CASCADE удалит регионы автоматически
    const char *sql = "DELETE FROM country WHERE id = ?;";
    return db_prepare_exec(db, sql, "i", id) == SQLITE_DONE;
}

// Callback для получения регионов
static int get_regions_callback(void *data, int argc, char **argv, char **azColName) {
    Region **regions = (Region**)data;
    int *count = (int*)(regions + 1);
    
    Region *r = &(*regions)[*count];
    r->id = atoi(argv[0]);
    strncpy(r->name, argv[1], 100);
    strncpy(r->capital, argv[2], 100);
    r->population = atoll(argv[3]);
    r->square = atof(argv[4]);
    r->country_id = atoi(argv[5]);
    
    (*count)++;
    return 0;
}

int country_get_regions(sqlite3 *db, int country_id, void **regions) {
    // Сначала считаем количество
    char sql_count[256];
    snprintf(sql_count, sizeof(sql_count), 
             "SELECT COUNT(*) FROM region WHERE country_id = %d;", country_id);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql_count, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    
    if (count == 0) {
        *regions = NULL;
        return 0;
    }
    
    // Выделяем память и получаем данные
    *regions = calloc(count, sizeof(Region));
    if (!*regions) return -1;
    
    char sql[512];
    snprintf(sql, sizeof(sql),
             "SELECT id, name, capital_region, population_region, square_region, country_id FROM region WHERE country_id = %d;",
             country_id);
    
    int idx = 0;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW && idx < count) {
        Region *r = &((Region*)*regions)[idx++];
        r->id = sqlite3_column_int(stmt, 0);
        strncpy(r->name, (const char*)sqlite3_column_text(stmt, 1), 100);
        strncpy(r->capital, (const char*)sqlite3_column_text(stmt, 2), 100);
        r->population = sqlite3_column_int64(stmt, 3);
        r->square = sqlite3_column_double(stmt, 4);
        r->country_id = sqlite3_column_int(stmt, 5);
    }
    sqlite3_finalize(stmt);
    
    return count;
}

double country_get_avg_region_population(sqlite3 *db, int country_id) {
    char sql[256];
    snprintf(sql, sizeof(sql),
             "SELECT AVG(population_region) FROM region WHERE country_id = %d;", country_id);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    
    double avg = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
        avg = sqlite3_column_double(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return avg;
}

void country_free(Country *country) {
    if (country && country->flag_image) {
        free(country->flag_image);
        country->flag_image = NULL;
    }
}
