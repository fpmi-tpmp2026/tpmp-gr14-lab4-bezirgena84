#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../includes/auth.h"
#include "../includes/database.h"
#include "../includes/country.h"
#include "../includes/region.h"

void print_menu(void) {
    printf("\n=== Country and Region Management System ===\n");
    printf("1. Add Country\n");
    printf("2. List Countries\n");
    printf("3. Add Region\n");
    printf("4. List Regions by Country\n");
    printf("5. Update Country\n");
    printf("6. Update Region\n");
    printf("7. Delete Country\n");
    printf("8. Delete Region\n");
    printf("9. Show Average Population by Country\n");
    printf("10. Show Total Population All Countries\n");
    printf("11. Logout\n");
    printf("0. Exit\n");
    printf("Choose an option: ");
}

int get_country_id_by_name(const char* name) {
    char sql[256];
    sprintf(sql, "SELECT id FROM countries WHERE name='%s'", name);
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    if (rc != SQLITE_OK) return -1;
    
    int id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        id = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return id;
}

int main(void) {
    printf("=== Country and Region Management System ===\n\n");
    
    if (!init_database()) {
        fprintf(stderr, "Failed to initialize database\n");
        return 1;
    }
    
    // Authentication
    char username[50], password[50];
    int attempts = 0;
    bool logged_in = false;
    
    while (attempts < 3 && !logged_in) {
        printf("Username: ");
        scanf("%49s", username);
        printf("Password: ");
        scanf("%49s", password);
        
        if (authenticate(username, password)) {
            printf("Welcome, %s!\n", username);
            logged_in = true;
        } else {
            printf("Invalid credentials. %d attempts remaining.\n", 2 - attempts);
            attempts++;
        }
    }
    
    if (!logged_in) {
        printf("Too many failed attempts. Exiting.\n");
        close_database();
        return 1;
    }
    
    // Main loop
    int choice;
    char name[100], capital[100], flag_image[256];
    long long population;
    int country_id, region_id;
    float area;
    
    do {
        print_menu();
        scanf("%d", &choice);
        getchar(); // Clear newline
        
        switch (choice) {
            case 1: // Add Country
                printf("Enter country name: ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0;
                printf("Enter capital: ");
                fgets(capital, sizeof(capital), stdin);
                capital[strcspn(capital, "\n")] = 0;
                printf("Enter population: ");
                scanf("%lld", &population);
                getchar();
                printf("Enter flag image path (optional): ");
                fgets(flag_image, sizeof(flag_image), stdin);
                flag_image[strcspn(flag_image, "\n")] = 0;
                
                add_country(name, capital, population, flag_image);
                break;
                
            case 2: // List Countries
                list_countries();
                break;
                
            case 3: // Add Region
                printf("Enter country name: ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0;
                country_id = get_country_id_by_name(name);
                
                if (country_id == -1) {
                    printf("Country not found!\n");
                    break;
                }
                
                printf("Enter region name: ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0;
                printf("Enter region capital: ");
                fgets(capital, sizeof(capital), stdin);
                capital[strcspn(capital, "\n")] = 0;
                printf("Enter population: ");
                scanf("%lld", &population);
                printf("Enter area (km²): ");
                scanf("%f", &area);
                getchar();
                printf("Enter map image path (optional): ");
                fgets(flag_image, sizeof(flag_image), stdin);
                flag_image[strcspn(flag_image, "\n")] = 0;
                
                add_region(country_id, name, capital, population, area, flag_image);
                break;
                
            case 4: // List Regions by Country
                printf("Enter country name: ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0;
                country_id = get_country_id_by_name(name);
                
                if (country_id == -1) {
                    printf("Country not found!\n");
                } else {
                    list_regions_by_country(country_id);
                }
                break;
                
            case 5: // Update Country
                printf("Enter country ID to update: ");
                scanf("%d", &country_id);
                getchar();
                printf("Enter new name: ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0;
                printf("Enter new capital: ");
                fgets(capital, sizeof(capital), stdin);
                capital[strcspn(capital, "\n")] = 0;
                printf("Enter new population: ");
                scanf("%lld", &population);
                
                update_country(country_id, name, capital, population);
                break;
                
            case 6: // Update Region
                printf("Enter region ID to update: ");
                scanf("%d", &region_id);
                getchar();
                printf("Enter new name: ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0;
                printf("Enter new capital: ");
                fgets(capital, sizeof(capital), stdin);
                capital[strcspn(capital, "\n")] = 0;
                printf("Enter new population: ");
                scanf("%lld", &population);
                printf("Enter new area: ");
                scanf("%f", &area);
                
                update_region(region_id, name, capital, population, area);
                break;
                
            case 7: // Delete Country
                printf("Enter country ID to delete: ");
                scanf("%d", &country_id);
                printf("Are you sure? (y/n): ");
                char confirm;
                scanf(" %c", &confirm);
                if (confirm == 'y' || confirm == 'Y') {
                    delete_country(country_id);
                }
                break;
                
            case 8: // Delete Region
                printf("Enter region ID to delete: ");
                scanf("%d", &region_id);
                printf("Are you sure? (y/n): ");
                char confirm2;
                scanf(" %c", &confirm2);
                if (confirm2 == 'y' || confirm2 == 'Y') {
                    delete_region(region_id);
                }
                break;
                
            case 9: // Show Average Population by Country
                printf("Enter country name: ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0;
                country_id = get_country_id_by_name(name);
                
                if (country_id == -1) {
                    printf("Country not found!\n");
                } else {
                    double avg = get_average_population_by_country(country_id);
                    if (avg > 0) {
                        printf("Average population in %s: %.2f\n", name, avg);
                    } else {
                        printf("No regions found for this country.\n");
                    }
                }
                break;
                
            case 10: // Show Total Population
                printf("Total population of all countries: %lld\n", 
                       get_total_population_all_countries());
                break;
                
            case 11: // Logout
                logout();
                printf("Logged out successfully!\n");
                close_database();
                printf("Exiting...\n");
                return 0;
                
            case 0: // Exit
                printf("Goodbye!\n");
                break;
                
            default:
                printf("Invalid option!\n");
        }
        
    } while (choice != 0);
    
    close_database();
    return 0;
}