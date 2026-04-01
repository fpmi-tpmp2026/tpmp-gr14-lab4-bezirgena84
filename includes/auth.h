#ifndef AUTH_H
#define AUTH_H

#include <stdbool.h>

typedef struct {
    char username[50];
    char password[64]; // Stored as hash
    int is_admin;
} User;

bool authenticate(const char* username, const char* password);
bool is_authenticated(void);
void logout(void);
void set_current_user(const char* username);
const char* get_current_user(void);
int get_current_user_is_admin(void);

#endif