#include "auth.h"
#include <string.h>
#include <openssl/sha.h>

void auth_hash_password(const char *password, char *hash) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), digest);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&hash[i*2], "%02x", digest[i]);
    }
    hash[SHA256_DIGEST_LENGTH * 2] = '\0';
}

bool auth_login(sqlite3 *db, const char *username, const char *password, User *user) {
    char hash[65];
    auth_hash_password(password, hash);
    
    const char *sql = "SELECT id, username, password_hash FROM user WHERE username = ? AND password_hash = ?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hash, -1, SQLITE_STATIC);
    
    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user->id = sqlite3_column_int(stmt, 0);
        strncpy(user->username, (const char*)sqlite3_column_text(stmt, 1), 50);
        strncpy(user->password_hash, (const char*)sqlite3_column_text(stmt, 2), 128);
        success = true;
    }
    
    sqlite3_finalize(stmt);
    return success;
}

bool auth_register(sqlite3 *db, const char *username, const char *password) {
    char hash[65];
    auth_hash_password(password, hash);
    
    const char *sql = "INSERT INTO user (username, password_hash) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hash, -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

void auth_clear_user(User *user) {
    if (user) {
        memset(user, 0, sizeof(User));
    }
}
