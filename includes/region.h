#ifndef REGION_H
#define REGION_H

#include <sqlite3.h>
#include <stdbool.h>

// Структура региона
typedef struct {
    int id;
    char name[101];
    char capital[101];
    long long population;
    double square;
    int country_id;
    char country_name[101];  // Для отображения
} Region;

// Создание нового региона
bool region_create(sqlite3 *db, Region *region);

// Получение региона по ID
bool region_get_by_id(sqlite3 *db, int id, Region *region);

// Получение всех регионов
int region_get_all(sqlite3 *db, Region **regions);

// Получение регионов по стране
int region_get_by_country(sqlite3 *db, int country_id, Region **regions);

// Обновление региона
bool region_update(sqlite3 *db, Region *region);

// Удаление региона
bool region_delete(sqlite3 *db, int id);

// Освобождение памяти региона
void region_free(Region *region);

#endif // REGION_H
