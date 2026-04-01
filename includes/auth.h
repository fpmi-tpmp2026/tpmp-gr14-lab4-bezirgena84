#ifndef AUTH_H
#define AUTH_H

#include <sqlite3.h>
#include <stdbool.h>

// Структура пользователя
typedef struct {
    int id;
    char username[51];
    char password_hash[129];
} User;

// Аутентификация пользователя
bool auth_login(sqlite3 *db, const char *username, const char *password, User *user);

// Регистрация нового пользователя
bool auth_register(sqlite3 *db, const char *username, const char *password);

// Простая хэш-функция для паролей (для демонстрации)
void auth_hash_password(const char *password, char *hash);

// Очистка данных пользователя
void auth_clear_user(User *user);

#endif // AUTH_H
