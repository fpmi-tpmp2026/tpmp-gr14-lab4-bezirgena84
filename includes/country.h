#ifndef COUNTRY_H
#define COUNTRY_H

#include <sqlite3.h>
#include <stdbool.h>

// Структура страны
typedef struct {
    int id;
    char name[101];
    char capital[101];
    char language[51];
    long long population;
    double square;
    char currency[4];
    char head[101];
    unsigned char *flag_image;  // BLOB для изображения флага
    size_t flag_size;
} Country;

// Создание новой страны
bool country_create(sqlite3 *db, Country *country);

// Получение страны по ID
bool country_get_by_id(sqlite3 *db, int id, Country *country);

// Получение всех стран
int country_get_all(sqlite3 *db, Country **countries);

// Обновление страны
bool country_update(sqlite3 *db, Country *country);

// Удаление страны (каскадно удаляет регионы)
bool country_delete(sqlite3 *db, int id);

// Получение списка регионов для страны
int country_get_regions(sqlite3 *db, int country_id, void **regions);

// Расчет среднего населения по регионам страны
double country_get_avg_region_population(sqlite3 *db, int country_id);

// Освобождение памяти страны
void country_free(Country *country);

#endif // COUNTRY_H
