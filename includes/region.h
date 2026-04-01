#ifndef REGION_H
#define REGION_H

#include <stdbool.h>

typedef struct {
    int id;
    int country_id;
    char name[100];
    char capital_city[100];
    long long population;
    float area;
    char map_image[256]; // Path to map image file
} Region;

bool add_region(int country_id, const char* name, const char* capital_city, 
                long long population, float area, const char* map_image);
bool delete_region(int region_id);
bool update_region(int region_id, const char* name, const char* capital_city, 
                   long long population, float area);
void list_regions_by_country(int country_id);
double get_average_population_by_country(int country_id);
Region* get_region_by_id(int region_id);
void free_region(Region* region);

#endif