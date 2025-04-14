
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

typedef struct CheckoutRecord {
    char member_id[50];
    char due_date[20];
    struct CheckoutRecord* next;
} CheckoutRecord;

typedef struct Book {
    int id;
    char title[100];
    char author[100];
    int total_copies;
    int available_copies;
    CheckoutRecord* checkouts;
    struct Book* next;
} Book;

typedef struct Reservation {
    char member_id[50];
    struct Reservation* next;
} Reservation;

Book* book_head = NULL;
Reservation* reservation_head = NULL;

Book* create_book(int id, char title[], char author[], int copies) {
    Book* new_book = (Book*)malloc(sizeof(Book));
    new_book->id = id;
    strcpy(new_book->title, title);
    strcpy(new_book->author, author);
    new_book->total_copies = copies;
    new_book->available_copies = copies;
    new_book->checkouts = NULL;
    new_book->next = NULL;
    return new_book;
}

void add_book() {
    int id, copies;
    char title[100], author[100];

    printf("Enter Book ID: ");
    scanf("%d", &id);
    printf("Enter Title: ");
    getchar();
    fgets(title, 100, stdin);
    title[strcspn(title, "\n")] = '\0';
    printf("Enter Author: ");
    fgets(author, 100, stdin);
    author[strcspn(author, "\n")] = '\0';
    printf("Enter Total Copies: ");
    scanf("%d", &copies);

    Book* new_book = create_book(id, title, author, copies);
    new_book->next = book_head;
    book_head = new_book;
    printf("Book added successfully!\n");
}

CheckoutRecord* create_checkout(char member_id[], char due_date[]) {
    CheckoutRecord* new_checkout = (CheckoutRecord*)malloc(sizeof(CheckoutRecord));
    strcpy(new_checkout->member_id, member_id);
    strcpy(new_checkout->due_date, due_date);
    new_checkout->next = NULL;
    return new_checkout;
}

void checkout_book() {
    int book_id;
    char member_id[50], due_date[20];

    printf("Enter Book ID: ");
    scanf("%d", &book_id);
    printf("Enter Member ID: ");
    scanf("%s", member_id);
    printf("Enter Due Date (DD-MM-YYYY): ");
    scanf("%s", due_date);

    Book* current = book_head;
    while (current != NULL) {
        if (current->id == book_id) {
            if (current->available_copies > 0) {
                CheckoutRecord* new_checkout = create_checkout(member_id, due_date);
                new_checkout->next = current->checkouts;
                current->checkouts = new_checkout;
                current->available_copies--;
                printf("Book checked out successfully!\n");
                return;
            } else {
                printf("All copies checked out. Would you like to reserve? (y/n): ");
                char choice;
                scanf(" %c", &choice);
                if (tolower(choice) == 'y') {
                    Reservation* new_reservation = (Reservation*)malloc(sizeof(Reservation));
                    strcpy(new_reservation->member_id, member_id);
                    new_reservation->next = reservation_head;
                    reservation_head = new_reservation;
                    printf("Book reserved!\n");
                }
                return;
            }
        }
        current = current->next;
    }
    printf("Book not found!\n");
}

void return_book() {
    int book_id;
    char member_id[50];

    printf("Enter Book ID: ");
    scanf("%d", &book_id);
    printf("Enter Member ID: ");
    scanf("%s", member_id);

    Book* current = book_head;
    while (current != NULL) {
        if (current->id == book_id) {
            CheckoutRecord* temp = current->checkouts;
            CheckoutRecord* prev = NULL;

            while (temp != NULL) {
                if (strcmp(temp->member_id, member_id) == 0) {
                    if (prev == NULL) {
                        current->checkouts = temp->next;
                    } else {
                        prev->next = temp->next;
                    }
                    free(temp);
                    current->available_copies++;
                    printf("Book returned successfully!\n");

                    if (reservation_head != NULL) {
                        printf("Notify %s: Book is available now!\n", reservation_head->member_id);
                        Reservation* to_remove = reservation_head;
                        reservation_head = reservation_head->next;
                        free(to_remove);
                    }
                    return;
                }
                prev = temp;
                temp = temp->next;
            }
            printf("No active checkout found for this member!\n");
            return;
        }
        current = current->next;
    }
    printf("Book not found!\n");
}

void search_books() {
    char query[100];
    printf("Search by Title/Author: ");
    getchar();
    fgets(query, 100, stdin);
    query[strcspn(query, "\n")] = '\0';

    Book* current = book_head;
    while (current != NULL) {
        if (strstr(current->title, query) != NULL || strstr(current->author, query) != NULL) {
            printf("\nID: %d\nTitle: %s\nAuthor: %s\nAvailable: %d/%d\n",
                   current->id, current->title, current->author,
                   current->available_copies, current->total_copies);
        }
        current = current->next;
    }
}

int calculate_days_overdue(const char* due_date_str) {
    struct tm due_date = {0};
    sscanf(due_date_str, "%d-%d-%d", &due_date.tm_mday, &due_date.tm_mon, &due_date.tm_year);
    due_date.tm_mon -= 1;
    due_date.tm_year -= 1900;

    time_t due_time = mktime(&due_date);
    time_t now = time(NULL);

    double seconds = difftime(now, due_time);
    int days = (int)(seconds / (60 * 60 * 24));
    return (days > 0) ? days : 0;
}

void print_report() {
    printf("\n=== Library Report ===\n");
    Book* current = book_head;
    while (current != NULL) {
        printf("\nID: %d\nTitle: %s\nAuthor: %s\nAvailable: %d/%d\nCheckouts:\n",
               current->id, current->title, current->author,
               current->available_copies, current->total_copies);

        CheckoutRecord* checkout = current->checkouts;
        while (checkout != NULL) {
            int overdue_days = calculate_days_overdue(checkout->due_date);
            if (overdue_days > 0) {
                printf("- Member: %s, Due: %s [OVERDUE by %d days, Fine: $%d]\n",
                       checkout->member_id, checkout->due_date, overdue_days, overdue_days);
            } else {
                printf("- Member: %s, Due: %s\n", checkout->member_id, checkout->due_date);
            }
            checkout = checkout->next;
        }
        current = current->next;
    }

    printf("\nActive Reservations:\n");
    Reservation* res = reservation_head;
    while (res != NULL) {
        printf("- Member: %s\n", res->member_id);
        res = res->next;
    }
}

int main() {
    int choice;
    do {
        printf("\nLibrary Management System\n");
        printf("1. Add Book\n2. Search Books\n3. Checkout Book\n4. Return Book\n5. Generate Report\n6. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch(choice) {
            case 1: add_book(); break;
            case 2: search_books(); break;
            case 3: checkout_book(); break;
            case 4: return_book(); break;
            case 5: print_report(); break;
            case 6: printf("Exiting...\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while(choice != 6);

    return 0;
}
