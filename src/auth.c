#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <openssl/evp.h>
#include "../includes/auth.h"
#include "../includes/database.h"

static User current_user = {0};
static bool authenticated = false;

void hash_password(const char* password, char* output) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_sha256();
    
    EVP_DigestInit_ex(ctx, md, NULL);
    EVP_DigestUpdate(ctx, password, strlen(password));
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    
    EVP_MD_CTX_free(ctx);
    
    for(unsigned int i = 0; i < hash_len; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[hash_len * 2] = '\0';
}

bool authenticate(const char* username, const char* password) {
    char sql[512];
    char hashed_pass[65];
    hash_password(password, hashed_pass);
    
    sprintf(sql, "SELECT username, is_admin FROM users WHERE username='%s' AND password='%s'", 
            username, hashed_pass);
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    if (rc != SQLITE_OK) {
        return false;
    }
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        strcpy(current_user.username, (const char*)sqlite3_column_text(stmt, 0));
        current_user.is_admin = sqlite3_column_int(stmt, 1);
        authenticated = true;
        sqlite3_finalize(stmt);
        return true;
    }
    
    sqlite3_finalize(stmt);
    return false;
}

bool is_authenticated(void) {
    return authenticated;
}

void logout(void) {
    authenticated = false;
    memset(&current_user, 0, sizeof(User));
}

void set_current_user(const char* username) {
    strcpy(current_user.username, username);
}

const char* get_current_user(void) {
    return current_user.username;
}

int get_current_user_is_admin(void) {
    return current_user.is_admin;
}