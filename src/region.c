#include "region.h"
#include <string.h>

bool region_create(sqlite3 *db, Region *region) {
    const char *sql = "INSERT INTO region (name, capital_region, population_region, square_region, country_id) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, region->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, region->capital, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, region->population);
    sqlite3_bind_double(stmt, 4, region->square);
    sqlite3_bind_int(stmt, 5, region->country_id);
    
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        region->id = sqlite3_last_insert_rowid(db);
    }
    
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool region_get_by_id(sqlite3 *db, int id, Region *region) {
    const char *sql = "SELECT id, name, capital_region, population_region, square_region, country_id FROM region WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        region->id = sqlite3_column_int(stmt, 0);
        strncpy(region->name, (const char*)sqlite3_column_text(stmt, 1), 100);
        strncpy(region->capital, (const char*)sqlite3_column_text(stmt, 2), 100);
        region->population = sqlite3_column_int64(stmt, 3);
        region->square = sqlite3_column_double(stmt, 4);
        region->country_id = sqlite3_column_int(stmt, 5);
        found = true;
    }
    
    sqlite3_finalize(stmt);
    return found;
}

int region_get_all(sqlite3 *db, Region **regions) {
    const char *sql = "SELECT r.id, r.name, r.capital_region, r.population_region, r.square_region, r.country_id, c.name FROM region r JOIN country c ON r.country_id = c.id;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) count++;
    sqlite3_reset(stmt);
    
    if (count == 0) {
        sqlite3_finalize(stmt);
        *regions = NULL;
        return 0;
    }
    
    *regions = calloc(count, sizeof(Region));
    if (!*regions) {
        sqlite3_finalize(stmt);
        return -1;
    }
    
    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < count) {
        Region *r = &(*regions)[i++];
        r->id = sqlite3_column_int(stmt, 0);
        strncpy(r->name, (const char*)sqlite3_column_text(stmt, 1), 100);
        strncpy(r->capital, (const char*)sqlite3_column_text(stmt, 2), 100);
        r->population = sqlite3_column_int64(stmt, 3);
        r->square = sqlite3_column_double(stmt, 4);
        r->country_id = sqlite3_column_int(stmt, 5);
        strncpy(r->country_name, (const char*)sqlite3_column_text(stmt, 6), 100);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

int region_get_by_country(sqlite3 *db, int country_id, Region **regions) {
    char sql[512];
    snprintf(sql, sizeof(sql),
             "SELECT id, name, capital_region, population_region, square_region, country_id FROM region WHERE country_id = %d;",
             country_id);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) count++;
    sqlite3_reset(stmt);
    
    if (count == 0) {
        sqlite3_finalize(stmt);
        *regions = NULL;
        return 0;
    }
    
    *regions = calloc(count, sizeof(Region));
    if (!*regions) {
        sqlite3_finalize(stmt);
        return -1;
    }
    
    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < count) {
        Region *r = &(*regions)[i++];
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

bool region_update(sqlite3 *db, Region *region) {
    const char *sql = "UPDATE region SET name=?, capital_region=?, population_region=?, square_region=?, country_id=? WHERE id=?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, region->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, region->capital, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, region->population);
    sqlite3_bind_double(stmt, 4, region->square);
    sqlite3_bind_int(stmt, 5, region->country_id);
    sqlite3_bind_int(stmt, 6, region->id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool region_delete(sqlite3 *db, int id) {
    const char *sql = "DELETE FROM region WHERE id = ?;";
    return db_prepare_exec(db, sql, "i", id) == SQLITE_DONE;
}

void region_free(Region *region) {
    // Нет динамической памяти в структуре
    (void)region;
}
