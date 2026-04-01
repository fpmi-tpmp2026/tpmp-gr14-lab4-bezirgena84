#ifndef COUNTRY_H
#define COUNTRY_H

#include <stdbool.h>

typedef struct {
    int id;
    char name[100];
    char capital[100];
    long long population;
    char flag_image[256]; // Path to flag image file
} Country;

bool add_country(const char* name, const char* capital, long long population, const char* flag_image);
bool delete_country(int country_id);
bool update_country(int country_id, const char* name, const char* capital, long long population);
void list_countries(void);
Country* get_country_by_id(int country_id);
void free_country(Country* country);
long long get_total_population_all_countries(void);

#endif