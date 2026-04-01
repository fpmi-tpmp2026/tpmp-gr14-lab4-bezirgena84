#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "../includes/region.h"
#include "../includes/database.h"

bool add_region(int country_id, const char* name, const char* capital_city, 
                long long population, float area, const char* map_image) {
    sqlite3_stmt* stmt;
    
    const char* insert_sql = "INSERT INTO regions (country_id, name, capital_city, population, area, map_image) VALUES (?, ?, ?, ?, ?, ?)";
    int rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, country_id);
    sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, capital_city, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 4, population);
    sqlite3_bind_double(stmt, 5, area);
    sqlite3_bind_text(stmt, 6, map_image, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert region: %s\n", sqlite3_errmsg(db));
        return false;
    }
    
    printf("Region '%s' added successfully!\n", name);
    return true;
}

bool delete_region(int region_id) {
    char sql[256];
    sprintf(sql, "DELETE FROM regions WHERE id = %d", region_id);
    
    if (execute_query(sql)) {
        printf("Region with ID %d deleted successfully!\n", region_id);
        return true;
    }
    return false;
}

bool update_region(int region_id, const char* name, const char* capital_city, 
                   long long population, float area) {
    char sql[1024];
    sprintf(sql, "UPDATE regions SET name='%s', capital_city='%s', population=%lld, area=%.2f WHERE id=%d",
            name, capital_city, population, area, region_id);
    
    if (execute_query(sql)) {
        printf("Region with ID %d updated successfully!\n", region_id);
        return true;
    }
    return false;
}

void list_regions_by_country(int country_id) {
    char sql[512];
    sprintf(sql, "SELECT id, name, capital_city, population, area FROM regions WHERE country_id=%d", country_id);
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    printf("\n=== Regions ===\n");
    printf("%-5s %-30s %-20s %-15s %-10s\n", "ID", "Name", "Capital", "Population", "Area (km²)");
    printf("----------------------------------------------------------------\n");
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        const char* capital = (const char*)sqlite3_column_text(stmt, 2);
        long long population = sqlite3_column_int64(stmt, 3);
        double area = sqlite3_column_double(stmt, 4);
        
        printf("%-5d %-30s %-20s %-15lld %-10.2f\n", id, name, capital, population, area);
    }
    printf("\n");
    
    sqlite3_finalize(stmt);
}

double get_average_population_by_country(int country_id) {
    char sql[512];
    sprintf(sql, "SELECT AVG(population) FROM regions WHERE country_id=%d", country_id);
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    if (rc != SQLITE_OK) {
        return 0;
    }
    
    double avg = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        avg = sqlite3_column_double(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return avg;
}

Region* get_region_by_id(int region_id) {
    char sql[256];
    sprintf(sql, "SELECT id, country_id, name, capital_city, population, area, map_image FROM regions WHERE id=%d", region_id);
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    if (rc != SQLITE_OK) {
        return NULL;
    }
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Region* region = (Region*)malloc(sizeof(Region));
        region->id = sqlite3_column_int(stmt, 0);
        region->country_id = sqlite3_column_int(stmt, 1);
        strcpy(region->name, (const char*)sqlite3_column_text(stmt, 2));
        strcpy(region->capital_city, (const char*)sqlite3_column_text(stmt, 3));
        region->population = sqlite3_column_int64(stmt, 4);
        region->area = (float)sqlite3_column_double(stmt, 5);
        const char* map = (const char*)sqlite3_column_text(stmt, 6);
        if (map) strcpy(region->map_image, map);
        else region->map_image[0] = '\0';
        
        sqlite3_finalize(stmt);
        return region;
    }
    
    sqlite3_finalize(stmt);
    return NULL;
}

void free_region(Region* region) {
    if (region) free(region);
}