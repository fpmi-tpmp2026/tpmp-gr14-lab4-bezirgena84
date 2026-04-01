#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "database.h"
#include "auth.h"
#include "country.h"
#include "region.h"

#define DB_FILE "countries.db"

// Глобальные переменные
static sqlite3 *g_db = NULL;
static User g_current_user = {0};
static bool g_authenticated = false;

// Прототипы функций меню
void show_main_menu();
void show_auth_menu();
void show_country_menu();
void show_region_menu();
void show_reports_menu();

// Функции аутентификации
void do_login();
void do_register();
void do_logout();

// Функции работы со странами
void do_create_country();
void do_list_countries();
void do_update_country();
void do_delete_country();

// Функции работы с регионами
void do_create_region();
void do_list_regions();
void do_update_region();
void do_delete_region();

// Функции отчетов (SELECT запросы)
void do_show_regions_by_country();
void do_show_avg_population_by_country();
void do_show_total_population();

// Вспомогательные функции
void clear_input_buffer();
int get_int_input(const char *prompt);
void get_string_input(const char *prompt, char *buffer, size_t size);

int main() {
    printf("=== Country Management System ===\n\n");
    
    // Инициализация БД
    if (db_init(&g_db, DB_FILE) != SQLITE_OK) {
        fprintf(stderr, "Не удалось подключиться к базе данных!\n");
        return 1;
    }
    
    // Создание таблиц, если не существуют
    db_execute(g_db, 
        "CREATE TABLE IF NOT EXISTS user ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username VARCHAR(50) NOT NULL UNIQUE,"
        "password_hash VARCHAR(128) NOT NULL"
        ");"
    );
    
    db_execute(g_db,
        "CREATE TABLE IF NOT EXISTS country ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name VARCHAR(100) NOT NULL,"
        "capital VARCHAR(100),"
        "language VARCHAR(50),"
        "population_country INTEGER,"
        "square_country REAL,"
        "currency VARCHAR(3),"
        "head_country VARCHAR(100),"
        "flag_image BLOB"
        ");"
    );
    
    db_execute(g_db,
        "CREATE TABLE IF NOT EXISTS region ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name VARCHAR(100) NOT NULL,"
        "capital_region VARCHAR(100),"
        "population_region INTEGER,"
        "square_region REAL,"
        "country_id INTEGER NOT NULL,"
        "FOREIGN KEY (country_id) REFERENCES country(id) ON DELETE CASCADE"
        ");"
    );
    
    // Инициализация тестовых данных
    db_init_test_data(g_db);
    
    // Главный цикл приложения
    while (true) {
        if (g_authenticated) {
            show_main_menu();
        } else {
            show_auth_menu();
        }
    }
    
    // Очистка
    db_close(g_db);
    return 0;
}

void show_auth_menu() {
    printf("\n--- Авторизация ---\n");
    printf("1. Войти\n");
    printf("2. Регистрация\n");
    printf("0. Выход\n");
    printf("Выбор: ");
    
    int choice;
    scanf("%d", &choice);
    clear_input_buffer();
    
    switch (choice) {
        case 1: do_login(); break;
        case 2: do_register(); break;
        case 0: 
            printf("До свидания!\n");
            exit(0);
        default:
            printf("Неверный выбор!\n");
    }
}

void do_login() {
    char username[51], password[51];
    
    get_string_input("Логин: ", username, sizeof(username));
    get_string_input("Пароль: ", password, sizeof(password));
    
    if (auth_login(g_db, username, password, &g_current_user)) {
        g_authenticated = true;
        printf("✓ Вход выполнен успешно, %s!\n", g_current_user.username);
    } else {
        printf("✗ Неверный логин или пароль!\n");
    }
}

void do_register() {
    char username[51], password[51];
    
    get_string_input("Придумайте логин: ", username, sizeof(username));
    get_string_input("Придумайте пароль: ", password, sizeof(password));
    
    if (auth_register(g_db, username, password)) {
        printf("✓ Регистрация успешна! Теперь войдите.\n");
    } else {
        printf("✗ Ошибка регистрации (возможно, пользователь уже существует)!\n");
    }
}

void do_logout() {
    auth_clear_user(&g_current_user);
    g_authenticated = false;
    printf("✓ Вы вышли из системы.\n");
}

void show_main_menu() {
    printf("\n=== Главное меню (Пользователь: %s) ===\n", g_current_user.username);
    printf("1. Управление странами (CRUD Countries)\n");
    printf("2. Управление регионами (CRUD Regions)\n");
    printf("3. Отчеты (SELECT запросы)\n");
    printf("4. Выход из системы (Logout)\n");
    printf("0. Выход из приложения\n");
    printf("Выбор: ");
    
    int choice;
    scanf("%d", &choice);
    clear_input_buffer();
    
    switch (choice) {
        case 1: show_country_menu(); break;
        case 2: show_region_menu(); break;
        case 3: show_reports_menu(); break;
        case 4: do_logout(); break;
        case 0: 
            printf("До свидания!\n");
            exit(0);
        default:
            printf("Неверный выбор!\n");
    }
}

void show_country_menu() {
    printf("\n--- Управление странами ---\n");
    printf("1. Добавить страну (INSERT)\n");
    printf("2. Показать все страны (SELECT)\n");
    printf("3. Обновить страну (UPDATE)\n");
    printf("4. Удалить страну (DELETE)\n");
    printf("0. Назад\n");
    printf("Выбор: ");
    
    int choice;
    scanf("%d", &choice);
    clear_input_buffer();
    
    switch (choice) {
        case 1: do_create_country(); break;
        case 2: do_list_countries(); break;
        case 3: do_update_country(); break;
        case 4: do_delete_country(); break;
        case 0: break;
        default: printf("Неверный выбор!\n");
    }
}

void do_create_country() {
    Country country = {0};
    
    printf("\n--- Добавление новой страны ---\n");
    get_string_input("Название: ", country.name, sizeof(country.name));
    get_string_input("Столица: ", country.capital, sizeof(country.capital));
    get_string_input("Язык: ", country.language, sizeof(country.language));
    country.population = get_int_input("Население: ");
    country.square = get_int_input("Площадь (км²): ");
    get_string_input("Валюта (код, 3 буквы): ", country.currency, sizeof(country.currency));
    get_string_input("Глава государства: ", country.head, sizeof(country.head));
    
    // Демонстрация работы с BLOB (загрузка "изображения")
    printf("Загрузить изображение флага? (y/n): ");
    char load_flag;
    scanf(" %c", &load_flag);
    clear_input_buffer();
    
    if (load_flag == 'y' || load_flag == 'Y') {
        // Для демонстрации создаем фиктивные данные изображения
        country.flag_size = 100;
        country.flag_image = malloc(country.flag_size);
        if (country.flag_image) {
            // Заполняем "изображение" тестовыми данными
            for (size_t i = 0; i < country.flag_size; i++) {
                country.flag_image[i] = (unsigned char)(i % 256);
            }
            printf("✓ Изображение флага загружено (%zu байт)\n", country.flag_size);
        }
    }
    
    if (country_create(g_db, &country)) {
        printf("✓ Страна '%s' успешно добавлена! ID: %d\n", country.name, country.id);
    } else {
        printf("✗ Ошибка добавления страны!\n");
    }
    
    country_free(&country);
}

void do_list_countries() {
    printf("\n--- Список стран ---\n");
    
    Country *countries = NULL;
    int count = country_get_all(g_db, &countries);
    
    if (count <= 0) {
        printf("Стран не найдено.\n");
        return;
    }
    
    printf("%-4s %-20s %-15s %-12s %-12s %-10s %-5s %-15s\n",
           "ID", "Название", "Столица", "Язык", "Население", "Площадь", "Вал.", "Глава");
    printf("------------------------------------------------------------------------\n");
    
    for (int i = 0; i < count; i++) {
        Country *c = &countries[i];
        printf("%-4d %-20s %-15s %-12s %-12lld %-12.0f %-5s %-15s\n",
               c->id, c->name, c->capital, c->language, 
               c->population, c->square, c->currency, c->head);
    }
    
    free(countries);
}

void do_update_country() {
    do_list_countries();
    
    int id = get_int_input("\nID страны для обновления: ");
    
    Country country;
    if (!country_get_by_id(g_db, id, &country)) {
        printf("✗ Страна с ID %d не найдена!\n", id);
        return;
    }
    
    printf("\n--- Обновление страны (оставьте пустым для пропуска) ---\n");
    printf("Текущее название: %s\n", country.name);
    char temp[101];
    get_string_input("Новое название: ", temp, sizeof(temp));
    if (strlen(temp) > 0) strncpy(country.name, temp, 100);
    
    printf("Текущая столица: %s\n", country.capital);
    get_string_input("Новая столица: ", temp, sizeof(temp));
    if (strlen(temp) > 0) strncpy(country.capital, temp, 100);
    
    printf("Текущий язык: %s\n", country.language);
    get_string_input("Новый язык: ", temp, sizeof(temp));
    if (strlen(temp) > 0) strncpy(country.language, temp, 50);
    
    printf("Текущее население: %lld\n", country.population);
    int pop = get_int_input("Новое население (0 для пропуска): ");
    if (pop > 0) country.population = pop;
    
    printf("Текущая площадь: %.0f км²\n", country.square);
    double sq = get_int_input("Новая площадь (0 для пропуска): ");
    if (sq > 0) country.square = sq;
    
    printf("Текущая валюта: %s\n", country.currency);
    get_string_input("Новая валюта: ", temp, sizeof(temp));
    if (strlen(temp) > 0) strncpy(country.currency, temp, 3);
    
    printf("Текущий глава: %s\n", country.head);
    get_string_input("Новый глава: ", temp, sizeof(temp));
    if (strlen(temp) > 0) strncpy(country.head, temp, 100);
    
    if (country_update(g_db, &country)) {
        printf("✓ Страна успешно обновлена!\n");
    } else {
        printf("✗ Ошибка обновления!\n");
    }
}

void do_delete_country() {
    do_list_countries();
    
    int id = get_int_input("\nID страны для удаления: ");
    
    // Подтверждение
    printf("⚠ Внимание: При удалении страны все её регионы также будут удалены!\n");
    printf("Подтвердить удаление? (y/n): ");
    char confirm;
    scanf(" %c", &confirm);
    clear_input_buffer();
    
    if (confirm == 'y' || confirm == 'Y') {
        if (country_delete(g_db, id)) {
            printf("✓ Страна и все её регионы успешно удалены!\n");
        } else {
            printf("✗ Ошибка удаления!\n");
        }
    } else {
        printf("Удаление отменено.\n");
    }
}

void show_region_menu() {
    printf("\n--- Управление регионами ---\n");
    printf("1. Добавить регион (INSERT)\n");
    printf("2. Показать все регионы (SELECT)\n");
    printf("3. Обновить регион (UPDATE)\n");
    printf("4. Удалить регион (DELETE)\n");
    printf("0. Назад\n");
    printf("Выбор: ");
    
    int choice;
    scanf("%d", &choice);
    clear_input_buffer();
    
    switch (choice) {
        case 1: do_create_region(); break;
        case 2: do_list_regions(); break;
        case 3: do_update_region(); break;
        case 4: do_delete_region(); break;
        case 0: break;
        default: printf("Неверный выбор!\n");
    }
}

void do_create_region() {
    // Сначала показываем страны для выбора
    printf("\n--- Выбор страны для региона ---\n");
    do_list_countries();
    
    int country_id = get_int_input("\nID страны: ");
    
    // Проверяем существование страны
    Country country;
    if (!country_get_by_id(g_db, country_id, &country)) {
        printf("✗ Страна с ID %d не найдена!\n", country_id);
        return;
    }
    
    Region region = {0};
    region.country_id = country_id;
    
    printf("\n--- Добавление региона в '%s' ---\n", country.name);
    get_string_input("Название региона: ", region.name, sizeof(region.name));
    get_string_input("Административный центр: ", region.capital, sizeof(region.capital));
    region.population = get_int_input("Население: ");
    region.square = get_int_input("Площадь (км²): ");
    
    if (region_create(g_db, &region)) {
        printf("✓ Регион '%s' успешно добавлен! ID: %d\n", region.name, region.id);
    } else {
        printf("✗ Ошибка добавления региона!\n");
    }
}

void do_list_regions() {
    printf("\n--- Список регионов ---\n");
    
    Region *regions = NULL;
    int count = region_get_all(g_db, &regions);
    
    if (count <= 0) {
        printf("Регионов не найдено.\n");
        return;
    }
    
    printf("%-4s %-25s %-15s %-12s %-12s %-10s %-20s\n",
           "ID", "Название", "Центр", "Население", "Площадь", "CountryID", "Страна");
    printf("--------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < count; i++) {
        Region *r = &regions[i];
        printf("%-4d %-25s %-15s %-12lld %-12.0f %-10d %-20s\n",
               r->id, r->name, r->capital, r->population, r->square, 
               r->country_id, r->country_name[0] ? r->country_name : "(N/A)");
    }
    
    free(regions);
}

void do_update_region() {
    do_list_regions();
    
    int id = get_int_input("\nID региона для обновления: ");
    
    Region region;
    if (!region_get_by_id(g_db, id, &region)) {
        printf("✗ Регион с ID %d не найден!\n", id);
        return;
    }
    
    printf("\n--- Обновление региона (оставьте пустым для пропуска) ---\n");
    char temp[101];
    
    printf("Название: %s\n", region.name);
    get_string_input("Новое название: ", temp, sizeof(temp));
    if (strlen(temp) > 0) strncpy(region.name, temp, 100);
    
    printf("Административный центр: %s\n", region.capital);
    get_string_input("Новый центр: ", temp, sizeof(temp));
    if (strlen(temp) > 0) strncpy(region.capital, temp, 100);
    
    printf("Население: %lld\n", region.population);
    long long pop = get_int_input("Новое население (0 для пропуска): ");
    if (pop > 0) region.population = pop;
    
    printf("Площадь: %.0f км²\n", region.square);
    double sq = get_int_input("Новая площадь (0 для пропуска): ");
    if (sq > 0) region.square = sq;
    
    if (region_update(g_db, &region)) {
        printf("✓ Регион успешно обновлен!\n");
    } else {
        printf("✗ Ошибка обновления!\n");
    }
}

void do_delete_region() {
    do_list_regions();
    
    int id = get_int_input("\nID региона для удаления: ");
    
    printf("Подтвердить удаление? (y/n): ");
    char confirm;
    scanf(" %c", &confirm);
    clear_input_buffer();
    
    if (confirm == 'y' || confirm == 'Y') {
        if (region_delete(g_db, id)) {
            printf("✓ Регион успешно удален!\n");
        } else {
            printf("✗ Ошибка удаления!\n");
        }
    } else {
        printf("Удаление отменено.\n");
    }
}

void show_reports_menu() {
    printf("\n--- Отчеты (SELECT запросы) ---\n");
    printf("1. Регионы указанной страны (SELECT по country_id)\n");
    printf("2. Среднее население по регионам страны (SELECT AVG)\n");
    printf("3. Общее население всех стран (SELECT SUM)\n");
    printf("0. Назад\n");
    printf("Выбор: ");
    
    int choice;
    scanf("%d", &choice);
    clear_input_buffer();
    
    switch (choice) {
        case 1: do_show_regions_by_country(); break;
        case 2: do_show_avg_population_by_country(); break;
        case 3: do_show_total_population(); break;
        case 0: break;
        default: printf("Неверный выбор!\n");
    }
}

void do_show_regions_by_country() {
    printf("\n--- Регионы страны ---\n");
    do_list_countries();
    
    int country_id = get_int_input("\nID страны: ");
    
    Region *regions = NULL;
    int count = region_get_by_country(g_db, country_id, &regions);
    
    if (count <= 0) {
        printf("Регионов не найдено для этой страны.\n");
        return;
    }
    
    printf("\n%-4s %-25s %-15s %-12s %-12s\n",
           "ID", "Название", "Центр", "Население", "Площадь");
    printf("------------------------------------------------------------\n");
    
    for (int i = 0; i < count; i++) {
        Region *r = &regions[i];
        printf("%-4d %-25s %-15s %-12lld %-12.0f\n",
               r->id, r->name, r->capital, r->population, r->square);
    }
    
    printf("\nВсего регионов: %d\n", count);
    free(regions);
}

void do_show_avg_population_by_country() {
    printf("\n--- Среднее население по регионам ---\n");
    do_list_countries();
    
    int country_id = get_int_input("\nID страны: ");
    
    double avg = country_get_avg_region_population(g_db, country_id);
    
    if (avg >= 0) {
        printf("\n✓ Среднее население по регионам: %.0f человек\n", avg);
    } else {
        printf("✗ Ошибка вычисления или регионы не найдены.\n");
    }
}

void do_show_total_population() {
    printf("\n--- Общее население всех стран ---\n");
    
    const char *sql = "SELECT SUM(population_country) FROM country;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("✗ Ошибка выполнения запроса!\n");
        return;
    }
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        long long total = sqlite3_column_int64(stmt, 0);
        printf("\n✓ Суммарное население всех стран: %'lld человек\n", total);
    }
    
    sqlite3_finalize(stmt);
}

// Вспомогательные функции
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int get_int_input(const char *prompt) {
    printf("%s", prompt);
    int value;
    while (scanf("%d", &value) != 1) {
        printf("Введите число: ");
        clear_input_buffer();
    }
    clear_input_buffer();
    return value;
}

void get_string_input(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin)) {
        // Удаляем символ новой строки
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}
