#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h> 

#define FILENAME "inventory.txt"
#define BILL_LOG "bills.txt"
#define SPECIAL_USER_FILE "special_users.txt"

// Predefined admin credentials
#define ADMIN_USERNAME "admin"
#define ADMIN_PASSWORD "admin123"

typedef struct {
    int id;
    char name[50];
    int quantity;
    float price;
    char expiry[15];
} Medicine;

typedef struct {
    char username[50];
    char password[50];
} SpecialUser;

float totalSales = 0;

void removeDuplicatesInInventory() {
    Medicine med;
    FILE *file = fopen(FILENAME, "r");
    FILE *temp = fopen("temp.txt", "w");

    if (file == NULL || temp == NULL) {
        printf("\033[31mError: Unable to access inventory file.\033[0m\n");
        if (temp != NULL) fclose(temp);
        if (file != NULL) fclose(file);
        return;
    }

   // printf("\033[36mChecking for duplicate IDs in inventory...\033[0m\n");

    // Hash map to track unique IDs (C doesn't have std::map, so use arrays)
    int uniqueIDs[1000] = {0}; // Assumes IDs are within 1 to 1000 for simplicity
    int idIndex = 0;           // Tracks how many IDs have been added

    while (fscanf(file, "%d %s %d %f %s", &med.id, med.name, &med.quantity, &med.price, med.expiry) != EOF) {
        int isDuplicate = 0;

        // Check if the ID is already in the unique list
        for (int i = 0; i < idIndex; i++) {
            if (uniqueIDs[i] == med.id) {
                isDuplicate = 1; // Duplicate found
                break;
            }
        }

        if (!isDuplicate) {
            // Add unique ID to the tracker
            uniqueIDs[idIndex++] = med.id;

            // Write the unique record to the temp file
            fprintf(temp, "%d %s %d %.2f %s\n", med.id, med.name, med.quantity, med.price, med.expiry);
        } else {
           // printf("\033[33mDuplicate ID %d found and removed.\033[0m\n", med.id);
        }
    }

    fclose(file);
    fclose(temp);

    // Replace original file with the cleaned temp file
    remove(FILENAME);
    rename("temp.txt", FILENAME);

   // printf("\033[32mInventory cleaned successfully. Duplicate IDs removed.\033[0m\n");
}

// Utility function to generate a random 5-digit serial number
int generateSerialNumber() {
    srand(time(NULL));
    return 10000 + rand() % 90000;
}

void clearScreen() {
    #ifdef _WIN32
        system("cls");  // Windows command to clear the screen
    #else
        system("clear"); // Unix/Linux/Mac command to clear the screen
    #endif
}
// Back option utility function
int backOption() {
    int choice;
    printf("\nPress 1 to go back to the previous menu: ");
    scanf("%d", &choice);
    if (choice == 1) {
        return 1; // Go back
    } else {
        printf("Invalid input. Returning to the previous menu.\n");
        return 1;
    }
}

// Admin and Special User Authentication
int authenticate(const char *username, const char *password) {
    if (strcmp(username, ADMIN_USERNAME) == 0 && strcmp(password, ADMIN_PASSWORD) == 0) {
        return 1; // Admin authenticated
    }

    FILE *file = fopen(SPECIAL_USER_FILE, "r");
    SpecialUser user;

    if (file == NULL) return 0;

    while (fscanf(file, "%s %s", user.username, user.password) != EOF) {
        if (strcmp(username, user.username) == 0 && strcmp(password, user.password) == 0) {
            fclose(file);
            return 2; // Special user authenticated
        }
    }

    fclose(file);
    return 0; // Authentication failed
}

// Register a Special User (Admin Only)
void registerSpecialUser() {
    SpecialUser user;
    clearScreen();
    FILE *file = fopen(SPECIAL_USER_FILE, "a");

    if (file == NULL) {
        printf("Error opening user file.\n");
        return;
    }

    printf("Enter new special user username: ");
    scanf("%s", user.username);
    printf("Enter new special user password: ");
    scanf("%s", user.password);

    fprintf(file, "%s %s\n", user.username, user.password);
    fclose(file);

    printf("Special user registered successfully!\n");
    if (backOption()) return;
}
// Function to check if a given Medicine ID exists in the inventory
int isMedicineIDExists(int id) {
    Medicine med;
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) return 0;

    while (fscanf(file, "%d %s %d %f %s", &med.id, med.name, &med.quantity, &med.price, med.expiry) != EOF) {
        if (med.id == id) {
            fclose(file);
            return 1; // ID exists
        }
    }

    fclose(file);
    return 0; // ID does not exist
}
// Function to validate the expiry date (returns 1 if expired, 0 otherwise)
int isMedicineExpired(const char *expiry) {
    struct tm expDate = {0};
    time_t now = time(NULL);
    struct tm *currentTime = localtime(&now);

    // Parse expiry date (dd-mm-yyyy format)
    sscanf(expiry, "%d-%d-%d", &expDate.tm_mday, &expDate.tm_mon, &expDate.tm_year);
    expDate.tm_mon -= 1;  // Months are 0-11 in struct tm
    expDate.tm_year -= 1900; // Years since 1900 in struct tm

    time_t expTime = mktime(&expDate);

    // Compare expiry date with current date
    return expTime < now;
}

// Add Medicine (with validation for duplicate ID)
void addMedicine() {
    Medicine med;
    FILE *file = fopen(FILENAME, "a");

    if (file == NULL) {
        printf("Error opening inventory file.\n");
        return;
    }

    while (1) {
        printf("Enter Medicine ID (or press 0 to go back): ");
        scanf("%d", &med.id);
        if (med.id == 0) {
            fclose(file);
            return;
        }

        if (isMedicineIDExists(med.id)) {
            printf("\033[31mError: Medicine with ID %d already exists. Please use a unique ID.\033[0m\n", med.id);
        } else {
            break;
        }
    }

    printf("Enter Medicine Name: ");
    scanf("%s", med.name);
    printf("Enter Quantity: ");
    scanf("%d", &med.quantity);
    printf("Enter Price: ");
    scanf("%f", &med.price);
    printf("Enter Expiry Date (dd-mm-yyyy): ");
    scanf("%s", med.expiry);

    fprintf(file, "%d %s %d %.2f %s\n", med.id, med.name, med.quantity, med.price, med.expiry);
    fclose(file);

    printf("\033[32mMedicine added successfully!\033[0m\n");
    if (backOption()) return;
}
// View Inventory
void viewInventory() {
    Medicine med;
    clearScreen();
    FILE *file = fopen(FILENAME, "r");

    if (file == NULL) {
        printf("No medicines in inventory.\n");
        if (backOption()) return;
    }
    printf("\n\033[1;34m---------------------- Inventory ----------------------\033[0m\n");
    printf("ID\tName\t\tQuantity\tPrice\t\tExpiry Date\n");
    printf("-----------------------------------------------------------------\n");
    while (fscanf(file, "%d %s %d %f %s", &med.id, med.name, &med.quantity, &med.price, med.expiry) != EOF) {
        printf("%d\t%-15s\t%d\t\t%.2f\t\t%s\n", med.id, med.name, med.quantity, med.price, med.expiry);
    }
    fclose(file);
    if (backOption()) return;
}

// Update Medicine
void updateMedicine() {
    int id, found = 0;
    Medicine med;
    FILE *file = fopen(FILENAME, "r");
    FILE *temp = fopen("temp.txt", "w");
    if (file == NULL || temp == NULL) {
        printf("Error opening file.\n");
        return;
    }
    printf("Enter Medicine ID to Update (or 0 to go back): ");
    scanf("%d", &id);

    if (id == 0) {
        fclose(file);
        fclose(temp);
        remove("temp.txt");
        return; // Back option
    }

    while (fscanf(file, "%d %s %d %f %s", &med.id, med.name, &med.quantity, &med.price, med.expiry) != EOF) {
        if (med.id == id) {
            found = 1;
            printf("\n--- Current Details ---\n");
            printf("ID: %d\nName: %s\nQuantity: %d\nPrice: %.2f\nExpiry Date: %s\n", med.id, med.name, med.quantity, med.price, med.expiry);

            printf("\nEnter New Name (or press ENTER to keep current): ");
            getchar(); // Clear input buffer
            char newName[50];
            fgets(newName, sizeof(newName), stdin);
            if (newName[0] != '\n') {
                sscanf(newName, "%s", med.name); // Update name if provided
            }

            printf("Enter New Quantity (or -1 to keep current): ");
            int newQuantity;
            scanf("%d", &newQuantity);
            if (newQuantity != -1) {
                med.quantity = newQuantity; // Update quantity if provided
            }

            printf("Enter New Price (or -1 to keep current): ");
            float newPrice;
            scanf("%f", &newPrice);
            if (newPrice != -1) {
                med.price = newPrice; // Update price if provided
            }

            printf("Enter New Expiry Date (dd-mm-yyyy) (or press ENTER to keep current): ");
            getchar(); // Clear input buffer
            char newExpiry[20];
            fgets(newExpiry, sizeof(newExpiry), stdin);
            if (newExpiry[0] != '\n') {
                sscanf(newExpiry, "%s", med.expiry); // Update expiry if provided
            }

            printf("\nMedicine updated successfully!\n");
        }
        fprintf(temp, "%d %s %d %.2f %s\n", med.id, med.name, med.quantity, med.price, med.expiry);
    }

    fclose(file);
    fclose(temp);

    if (found) {
        remove(FILENAME);
        rename("temp.txt", FILENAME);
    } else {
        remove("temp.txt");
        printf("\nMedicine not found.\n");
    }

    if (backOption()) return;
}

// Delete Medicine
void deleteMedicine() {
    int id, found = 0;
    Medicine med;
    FILE *file = fopen(FILENAME, "r");
    FILE *temp = fopen("temp.txt", "w");

    if (file == NULL || temp == NULL) {
        printf("Error opening file.\n");
        if (backOption()) return;
    }

    printf("Enter Medicine ID to Delete (or press 0 to go back): ");
    scanf("%d", &id);
    if (id == 0) {
        fclose(file);
        fclose(temp);
        remove("temp.txt");
        return;
    }

    while (fscanf(file, "%d %s %d %f %s", &med.id, med.name, &med.quantity, &med.price, med.expiry) != EOF) {
        if (med.id == id) {
            found = 1;
            printf("Medicine deleted successfully.\n");
        } else {
            fprintf(temp, "%d %s %d %.2f %s\n", med.id, med.name, med.quantity, med.price, med.expiry);
        }
    }

    fclose(file);
    fclose(temp);

    if (found) {
        remove(FILENAME);
        rename("temp.txt", FILENAME);
    } else {
        remove("temp.txt");
        printf("Medicine not found.\n");
    }

    if (backOption()) return;
}

void browseMedicine() {
    Medicine med;
    char searchTerm[50];
    int found = 0;

    FILE *file = fopen(FILENAME, "r");

    if (file == NULL) {
        printf("\033[31mError: Inventory file not found.\033[0m\n");
        return;
    }

    printf("\033[36m--- Browse Medicines ---\033[0m\n");
    printf("Enter Medicine Name to Search: ");
    scanf(" %[^\n]", searchTerm);  // Allows spaces in the search term

    printf("\033[32m%-10s %-20s %-10s %-10s %-15s\033[0m\n", "ID", "Name", "Quantity", "Price", "Expiry");

    // Loop through the inventory to find matching medicine names
    while (fscanf(file, "%d %s %d %f %s", &med.id, med.name, &med.quantity, &med.price, med.expiry) != EOF) {
        // Convert both the search term and the medicine name to lowercase to handle case insensitivity
        char lowerSearchTerm[50], lowerMedicineName[50];
        strcpy(lowerSearchTerm, searchTerm);
        strcpy(lowerMedicineName, med.name);

        // Convert both to lowercase for case-insensitive comparison
        for (int i = 0; lowerSearchTerm[i]; i++) lowerSearchTerm[i] = tolower(lowerSearchTerm[i]);
        for (int i = 0; lowerMedicineName[i]; i++) lowerMedicineName[i] = tolower(lowerMedicineName[i]);

        // Search for partial or full matches
        if (strstr(lowerMedicineName, lowerSearchTerm) != NULL) {
            printf("%-10d %-20s %-10d %-10.2f %-15s\n", med.id, med.name, med.quantity, med.price, med.expiry);
            found = 1;
        }
    }

    if (!found) {
        printf("\033[31mNo medicine found with the name '%s'.\033[0m\n", searchTerm);
    }

    fclose(file);
    if (backOption()) return;
}
// Generate Bill (with expiry validation)
void generateBill() {
    removeDuplicatesInInventory();
    Medicine med;
    int medID, quantity, found;
    float subtotal = 0, tax, total = 0;
    char customerName[50];
    int serialNumber = time(NULL) % 100000 + rand() % 90000; // Generate a unique serial number

    FILE *file = fopen(FILENAME, "r");
    FILE *temp = fopen("temp.txt", "w");
    FILE *billLog = fopen(BILL_LOG, "a");

    if (file == NULL || temp == NULL) {
        printf("\033[31mError: Inventory file not found.\033[0m\n");
        if (temp != NULL) fclose(temp);
        if (file != NULL) fclose(file);
        if (backOption()) return;
    }

    printf("\033[36m--- Generate Bill ---\033[0m\n");
    printf("Enter Customer Name: ");
    scanf("%s", customerName);

    printf("\n\033[34mBill Serial Number: %d\033[0m\n", serialNumber);
    printf("\033[32m%-10s %-20s %-10s %-10s %-15s\033[0m\n", "ID", "Name", "Quantity", "Price", "Expiry");

    // Write bill header to the log file
    if (billLog) {
        time_t now = time(NULL);
        char formattedDate[100];
        strftime(formattedDate, sizeof(formattedDate), "%Y-%m-%d %H:%M:%S", localtime(&now));
        fprintf(billLog, "\nSerial Number: %d\nCustomer Name: %s\nDate: %s\n", serialNumber, customerName, formattedDate);
        fprintf(billLog, "--------------------------------------------------------------------\n");
        fprintf(billLog, "%-10s %-20s %-10s %-10s %-15s\n", "ID", "Name", "Quantity", "Price", "Expiry");
    }

    while (1) {
        printf("\033[33mEnter Medicine ID (or 0 to finish): \033[0m");
        scanf("%d", &medID);
        if (medID == 0) break;

        printf("Enter Quantity: ");
        scanf("%d", &quantity);

        found = 0;
        rewind(file);

        while (fscanf(file, "%d %s %d %f %s", &med.id, med.name, &med.quantity, &med.price, med.expiry) != EOF) {
            if (med.id == medID) {
                found = 1;

                // Check if expired
                if (isMedicineExpired(med.expiry)) {
                    printf("\033[31mAlert: Medicine '%s' (ID: %d) is expired!\033[0m\n", med.name, med.id);
                    break;
                }

                // Check for stock
                if (quantity > med.quantity) {
                    printf("\033[31mError: Not enough stock available for '%s'. Available: %d\033[0m\n", med.name, med.quantity);
                    break;
                }

                // Update stock and calculate cost
                med.quantity -= quantity;
                float cost = med.price * quantity;
                subtotal += cost;

                // Display and log detailed information
                printf("%-10d %-20s %-10d %-10.2f %-15s\n", med.id, med.name, quantity, med.price, med.expiry);
                if (billLog) {
                    fprintf(billLog, "%-10d %-20s %-10d %-10.2f %-15s\n", med.id, med.name, quantity, med.price, med.expiry);
                }
            }

            // Write updated inventory to temp file
            fprintf(temp, "%d %s %d %.2f %s\n", med.id, med.name, med.quantity, med.price, med.expiry);
        }

        if (!found) {
            printf("\033[31mError: Medicine ID %d not found in inventory.\033[0m\n", medID);
        }
    }

    fclose(file);
    fclose(temp);

    if (subtotal > 0) {
        tax = subtotal * 0.025;  // Calculate 2.5% tax
        total = subtotal + tax; // Final total
        totalSales += total;    // Update total sales

        // Display and log summary
        printf("\n\033[36mSubtotal: %.2f\033[0m\n", subtotal);
        printf("\033[36mTax (2.5%%): %.2f\033[0m\n", tax);
        printf("\033[32mTotal Amount: %.2f\033[0m\n", total);

        if (billLog) {
            fprintf(billLog, "--------------------------------------------------------------------\n");
            fprintf(billLog, "Subtotal: %.2f\nTax (2.5%%): %.2f\nTotal: %.2f\n", subtotal, tax, total);
            fprintf(billLog, "--------------------------------------------------------------------\n\n");
            fclose(billLog);
        }

        // Update inventory file
        remove(FILENAME);
        rename("temp.txt", FILENAME);
    } else {
        printf("\033[31mNo items were added to the bill.\033[0m\n");
        fclose(temp);
        remove("temp.txt");
    }

    if (backOption()) return;
}
// View Total Sales
void viewTotalSales() {
    clearScreen();
    printf("Total Sales: %.2f\n", totalSales);
    if (backOption()) return;
}

// Search Bill by Serial Number
void searchBill() {
    int serial, found = 0;
    char line[256];
    clearScreen();
    FILE *file = fopen(BILL_LOG, "r");

    if (file == NULL) {
        printf("No bills found.\n");
        if (backOption()) return;
    }

    printf("Enter Bill Serial Number: ");
    scanf("%d", &serial);

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "Serial Number:")) {
            int billSerial;
            sscanf(line, "Serial Number: %d", &billSerial);
            if (billSerial == serial) {
                found = 1;
                printf("\n--- Bill Details ---\n");
                printf("%s", line);
                while (fgets(line, sizeof(line), file) && strlen(line) > 1) {
                    printf("%s", line);
                }
                break;
            }
        }
    }

    fclose(file);
    if (!found) {
        printf("Bill not found.\n");
    }

    if (backOption()) return;
}

// Main Menu
void adminMenu() {
    while (1) {
        
        int choice;
        clearScreen();
        removeDuplicatesInInventory();
        printf("\n\033[1;34m--- Admin Menu ---\033[0m\n");
        printf("1. Add Medicine\n");
        printf("2. View Inventory\n");
        printf("3. Update Inventory\n");
        printf("4. Delete Medicine\n");
        printf("5. Register Special User\n");
        printf("6. Generate Bill\n");
        printf("7. View Total Sales\n");
        printf("8. Search Bill\n");
        printf("9. Logout\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                addMedicine();
                break;
            case 2:
                viewInventory();
                break;
            case 3:
                updateMedicine();
                break;
            case 4:
                deleteMedicine();
                break;
            case 5:
                registerSpecialUser();
                break;
            case 6:
                generateBill();
                break;
            case 7:
                viewTotalSales();
                break;
            case 8:
                searchBill();
                break;
            case 9:
                return;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }
}

void specialUserMenu() {
    while (1) {
        int choice;
        clearScreen();
        removeDuplicatesInInventory();
        printf("\n\033[1;34m--- Special User Menu ---\033[0m\n");
        printf("1. View Inventory\n");
        printf("2. Generate Bill\n");
        printf("3. Search Bill\n");
        printf("4. View Total Sales\n");
        printf("5. Logout\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                viewInventory();
                break;
            case 2:
                generateBill();
                break;
            case 3:
                searchBill();
                break;
            case 4:
                viewTotalSales();
                break;
            case 5:
                return;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }
}

void customerMenu() {
    while (1) {
        int choice;
        clearScreen();
        removeDuplicatesInInventory();
        printf("\n\033[1;34m--- Customer Menu ---\033[0m\n");
        printf("1. Browse Medicine\n");
        printf("2. Generate Bill\n");
        printf("3. Search Bill\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                browseMedicine();
                break;
            case 2:
                generateBill();
                break;
            case 3:
                searchBill();
                break;
            case 4:
                return;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }
}

// Main Function
int main() {
    while (1) {
        char username[50], password[50];
        int role;
        removeDuplicatesInInventory();
        printf("\n\033[1;34m--- Medical Store Management System ---\033[0m\n");
        printf("1. Admin Login\n");
        printf("2. Special User Login\n");
        printf("3. Customer\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &role);

        switch (role) {
            case 1: // Admin Login
                printf("\033[1;32mEnter Admin Username: \033[0m");
                scanf("%s", username);
                printf("\033[1;32mEnter Admin Password: \033[0m");
                scanf("%s", password);
                if (authenticate(username, password) == 1) {
                    printf("\033[1;32mAdmin logged in successfully!\033[0m");
                    adminMenu();
                } else {
                    printf("\033[1;31mInvalid Admin credentials.\033[0m\n");
                }
                break;

            case 2: // Special User Login
                printf("Enter Special User Username: ");
                scanf("%s", username);
                printf("Enter Special User Password: ");
                scanf("%s", password);
                if (authenticate(username, password) == 2) {
                    printf("Special User logged in successfully!\n");
                    specialUserMenu();
                } else {
                    printf("Invalid Special User credentials.\n");
                }
                break;
            case 3: // Customer
                printf("Welcome Customer!\n");
                customerMenu();
                break;
            case 4: // Exit
                printf("Thank you for using the system. Goodbye!\n");
                exit(0);
            default:
                printf("Invalid choice. Try again.\n");
        }
    }
    return 0;
}
