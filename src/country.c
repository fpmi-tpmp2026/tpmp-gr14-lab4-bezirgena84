#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "../includes/country.h"
#include "../includes/database.h"

bool add_country(const char* name, const char* capital, long long population, const char* flag_image) {
    sqlite3_stmt* stmt;
    
    const char* insert_sql = "INSERT INTO countries (name, capital, population, flag_image) VALUES (?, ?, ?, ?)";
    int rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, capital, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, population);
    sqlite3_bind_text(stmt, 4, flag_image, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert country: %s\n", sqlite3_errmsg(db));
        return false;
    }
    
    printf("Country '%s' added successfully!\n", name);
    return true;
}

bool delete_country(int country_id) {
    char sql[256];
    sprintf(sql, "DELETE FROM countries WHERE id = %d", country_id);
    
    if (execute_query(sql)) {
        printf("Country with ID %d deleted successfully!\n", country_id);
        return true;
    }
    return false;
}

bool update_country(int country_id, const char* name, const char* capital, long long population) {
    char sql[1024];
    sprintf(sql, "UPDATE countries SET name='%s', capital='%s', population=%lld WHERE id=%d",
            name, capital, population, country_id);
    
    if (execute_query(sql)) {
        printf("Country with ID %d updated successfully!\n", country_id);
        return true;
    }
    return false;
}

void list_countries(void) {
    char sql[] = "SELECT id, name, capital, population FROM countries";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    printf("\n=== Countries ===\n");
    printf("%-5s %-30s %-20s %-15s\n", "ID", "Name", "Capital", "Population");
    printf("--------------------------------------------------------\n");
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        const char* capital = (const char*)sqlite3_column_text(stmt, 2);
        long long population = sqlite3_column_int64(stmt, 3);
        
        printf("%-5d %-30s %-20s %-15lld\n", id, name, capital, population);
    }
    printf("\n");
    
    sqlite3_finalize(stmt);
}

Country* get_country_by_id(int country_id) {
    char sql[256];
    sprintf(sql, "SELECT id, name, capital, population, flag_image FROM countries WHERE id=%d", country_id);
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    if (rc != SQLITE_OK) {
        return NULL;
    }
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Country* country = (Country*)malloc(sizeof(Country));
        country->id = sqlite3_column_int(stmt, 0);
        strcpy(country->name, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(country->capital, (const char*)sqlite3_column_text(stmt, 2));
        country->population = sqlite3_column_int64(stmt, 3);
        const char* flag = (const char*)sqlite3_column_text(stmt, 4);
        if (flag) strcpy(country->flag_image, flag);
        else country->flag_image[0] = '\0';
        
        sqlite3_finalize(stmt);
        return country;
    }
    
    sqlite3_finalize(stmt);
    return NULL;
}

long long get_total_population_all_countries(void) {
    char sql[] = "SELECT SUM(population) FROM countries";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    if (rc != SQLITE_OK) {
        return 0;
    }
    
    long long total = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_int64(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return total;
}

void free_country(Country* country) {
    if (country) free(country);
}