#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

// Add Medicine
void addMedicine() {
    Medicine med;
    clearScreen();
    FILE *file = fopen(FILENAME, "a");

    if (file == NULL) {
        printf("Error opening inventory file.\n");
        return;
    }

    printf("Enter Medicine ID (or press 0 to go back): ");
    scanf("%d", &med.id);
    if (med.id == 0) {
        fclose(file);
        return;
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

    printf("Medicine added successfully!\n");
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

// Generate Bill
void generateBill() {
    Medicine med;
    int id, quantity, found = 0;
    float total = 0, tax;
    int serial = generateSerialNumber();
    FILE *file = fopen(FILENAME, "r");
    FILE *temp = fopen("temp.txt", "w");
    FILE *bill = fopen(BILL_LOG, "a");

    if (file == NULL || temp == NULL || bill == NULL) {
        printf("Error opening file.\n");
        if (backOption()) return;
    }

    printf("Bill Serial Number: %d\n", serial);
    fprintf(bill, "Serial Number: %d\n", serial);
    printf("\n--- Enter Medicine Details for Purchase ---\n");

    printf("Enter Medicine ID and Quantity (Enter 0 to stop):\n");
    printf("-------------------------------------------------\n");
    printf("%-10s %-20s %-15s %-10s\n", "ID", "Name", "Expiry", "Price");
    printf("-------------------------------------------------\n");

    while (1) {
        printf("Medicine ID: ");
        scanf("%d", &id);
        if (id == 0) break;

        printf("Quantity: ");
        scanf("%d", &quantity);

        rewind(file); // Reset file pointer to the beginning of the inventory
        while (fscanf(file, "%d %s %d %f %s", &med.id, med.name, &med.quantity, &med.price, med.expiry) != EOF) {
            if (med.id == id && med.quantity >= quantity) {
                found = 1;
                med.quantity -= quantity;
                total += med.price * quantity;
                printf("%-10d %-20s %-15s %.2f\n", med.id, med.name, med.expiry, med.price * quantity);
                fprintf(bill, "%s %d %s %.2f\n", med.name, quantity, med.expiry, med.price * quantity);
            }
            fprintf(temp, "%d %s %d %.2f %s\n", med.id, med.name, med.quantity, med.price, med.expiry);
        }
    }

    tax = total * 0.025;
    total += tax;

    printf("\nSubtotal: %.2f\n", total - tax);
    printf("Tax (2.5%%): %.2f\n", tax);
    printf("Total Amount: %.2f\n", total);

    fprintf(bill, "Tax: %.2f\n", tax);
    fprintf(bill, "Total Amount: %.2f\n\n", total);

    fclose(file);
    fclose(temp);
    fclose(bill);

    if (found) {
        remove(FILENAME);
        rename("temp.txt", FILENAME);
        totalSales += total;
        printf("\nBill generated successfully! Check bill log for details.\n");
    } else {
        remove("temp.txt");
        printf("\nNo valid medicines were selected or insufficient stock.\n");
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
        printf("\n\033[1;34m--- Special User Menu ---\033[0m\n");
        printf("1. View Inventory\n");
        printf("2. Generate Bill\n");
        printf("3. Search Bill\n");
        printf("4. Logout\n");
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
        printf("\n\033[1;34m--- Customer Menu ---\033[0m\n");
        printf("1. View Inventory\n");
        printf("2. Generate Bill\n");
        printf("3. Search Bill\n");
        printf("4. Exit\n");
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
