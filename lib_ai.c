#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME_LEN 50
#define MAX_SUBJECT_LEN 30
#define MAX_BOOKS 100
#define MAX_STUDENTS 50

typedef struct {
    int id;
    char title[MAX_NAME_LEN];
    char subject[MAX_SUBJECT_LEN];
    int available;
} Book;

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    int borrowedBookId;
    char lastBorrowedSubject[MAX_SUBJECT_LEN];
} Student;

// Global Variables
Book books[MAX_BOOKS];
Student students[MAX_STUDENTS];
int bookCount = 0, studentCount = 0;

// Function Prototypes
void loadBooks();
void saveBooks();
void loadStudents();
void saveStudents();
void addBook();
void removeBook();
void issueBook();
void returnBook();
void viewBooks();
void viewStudents();
void recommendBooks(const char *subject);
int levenshtein(const char *s1, const char *s2);

// Utility Functions
int min(int a, int b, int c) {
    int m = a;
    if (b < m) m = b;
    if (c < m) m = c;
    return m;
}

// Convert string to lowercase
void toLowerCase(char *str) {
    for (int i = 0; str[i]; i++)
        str[i] = tolower(str[i]);
}

// Levenshtein Distance
int levenshtein(const char *s1, const char *s2) {
    int len1 = strlen(s1), len2 = strlen(s2);
    int matrix[len1 + 1][len2 + 1];

    for (int i = 0; i <= len1; i++) matrix[i][0] = i;
    for (int j = 0; j <= len2; j++) matrix[0][j] = j;

    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            if (s1[i-1] == s2[j-1])
                matrix[i][j] = matrix[i-1][j-1];
            else
                matrix[i][j] = 1 + min(matrix[i-1][j],
                                       matrix[i][j-1],
                                       matrix[i-1][j-1]);
        }
    }
    return matrix[len1][len2];
}

// ================= MAIN =================
int main() {
    int choice;

    loadBooks();
    loadStudents();

    while (1) {
        printf("\n=== Library Management System ===\n");
        printf("1. Add Book\n");
        printf("2. Remove Book\n");
        printf("3. Issue Book\n");
        printf("4. Return Book\n");
        printf("5. View Books\n");
        printf("6. View Students\n");
        printf("7. Exit\n");

        printf("Enter choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1: addBook(); break;
            case 2: removeBook(); break;
            case 3: issueBook(); break;
            case 4: returnBook(); break;
            case 5: viewBooks(); break;
            case 6: viewStudents(); break;
            case 7:
                saveBooks();
                saveStudents();
                printf("Data saved. Exiting...\n");
                exit(0);
            default:
                printf("Invalid choice!\n");
        }
    }
}

// ================= FILE HANDLING =================
void loadBooks() {
    FILE *f = fopen("books.dat", "rb");
    if (f) {
        fread(&bookCount, sizeof(int), 1, f);
        fread(books, sizeof(Book), bookCount, f);
        fclose(f);
    }
}

void saveBooks() {
    FILE *f = fopen("books.dat", "wb");
    if (f) {
        fwrite(&bookCount, sizeof(int), 1, f);
        fwrite(books, sizeof(Book), bookCount, f);
        fclose(f);
    }
}

void loadStudents() {
    FILE *f = fopen("students.dat", "rb");
    if (f) {
        fread(&studentCount, sizeof(int), 1, f);
        fread(students, sizeof(Student), studentCount, f);
        fclose(f);
    }
}

void saveStudents() {
    FILE *f = fopen("students.dat", "wb");
    if (f) {
        fwrite(&studentCount, sizeof(int), 1, f);
        fwrite(students, sizeof(Student), studentCount, f);
        fclose(f);
    }
}

// ================= BOOK FUNCTIONS =================
void addBook() {
    if (bookCount >= MAX_BOOKS) {
        printf("Book limit reached!\n");
        return;
    }

    Book b;

    printf("Enter Book ID: ");
    scanf("%d", &b.id);
    getchar();

    // Duplicate check
    for (int i = 0; i < bookCount; i++) {
        if (books[i].id == b.id) {
            printf("Book ID already exists!\n");
            return;
        }
    }

    printf("Enter Title: ");
    fgets(b.title, MAX_NAME_LEN, stdin);
    b.title[strcspn(b.title, "\n")] = '\0';

    printf("Enter Subject: ");
    fgets(b.subject, MAX_SUBJECT_LEN, stdin);
    b.subject[strcspn(b.subject, "\n")] = '\0';

    b.available = 1;

    books[bookCount++] = b;
    saveBooks();

    printf("Book added successfully!\n");
}

void removeBook() {
    int id, found = 0;

    printf("Enter Book ID: ");
    scanf("%d", &id);

    for (int i = 0; i < bookCount; i++) {
        if (books[i].id == id) {
            found = 1;
            for (int j = i; j < bookCount - 1; j++) {
                books[j] = books[j+1];
            }
            bookCount--;
            saveBooks();
            printf("Book removed!\n");
            return;
        }
    }

    if (!found) printf("Book not found!\n");
}

// ================= ISSUE / RETURN =================
void issueBook() {
    int sid, bid;
    char name[MAX_NAME_LEN];

    printf("Enter Student ID: ");
    scanf("%d", &sid);
    getchar();

    printf("Enter Student Name: ");
    fgets(name, MAX_NAME_LEN, stdin);
    name[strcspn(name, "\n")] = '\0';

    printf("Enter Book ID: ");
    scanf("%d", &bid);

    Book *bookPtr = NULL;

    for (int i = 0; i < bookCount; i++) {
        if (books[i].id == bid) {
            bookPtr = &books[i];
            break;
        }
    }

    if (!bookPtr) {
        printf("Book not found!\n");
        return;
    }

    if (!bookPtr->available) {
        printf("Book not available!\n");
        recommendBooks(bookPtr->subject);
        return;
    }

    bookPtr->available = 0;

    int found = 0;
    for (int i = 0; i < studentCount; i++) {
        if (students[i].id == sid) {
            students[i].borrowedBookId = bid;
            strcpy(students[i].lastBorrowedSubject, bookPtr->subject);
            found = 1;
            break;
        }
    }

    if (!found) {
        if (studentCount >= MAX_STUDENTS) {
            printf("Student limit reached!\n");
            return;
        }

        Student s = {0};
        s.id = sid;
        strcpy(s.name, name);
        s.borrowedBookId = bid;
        strcpy(s.lastBorrowedSubject, bookPtr->subject);

        students[studentCount++] = s;
    }

    saveBooks();
    saveStudents();

    printf("Book issued successfully!\n");

    printf("\nRecommended Books:\n");
    recommendBooks(bookPtr->subject);
}

void returnBook() {
    int sid, bid;

    printf("Enter Student ID: ");
    scanf("%d", &sid);

    printf("Enter Book ID: ");
    scanf("%d", &bid);

    for (int i = 0; i < studentCount; i++) {
        if (students[i].id == sid && students[i].borrowedBookId == bid) {

            students[i].borrowedBookId = -1;

            for (int j = 0; j < bookCount; j++) {
                if (books[j].id == bid) {
                    books[j].available = 1;
                    saveBooks();
                    saveStudents();
                    printf("Book returned!\n");
                    return;
                }
            }

            printf("Book record not found!\n");
            return;
        }
    }

    printf("Record not found!\n");
}

// ================= VIEW =================
void viewBooks() {
    printf("\n--- Books ---\n");

    for (int i = 0; i < bookCount; i++) {
        printf("ID:%d | %s | %s | %s\n",
               books[i].id,
               books[i].title,
               books[i].subject,
               books[i].available ? "Available" : "Issued");
    }
}

void viewStudents() {
    printf("\n--- Students ---\n");

    for (int i = 0; i < studentCount; i++) {
        printf("ID:%d | %s | Book:%d | Subject:%s\n",
               students[i].id,
               students[i].name,
               students[i].borrowedBookId,
               students[i].lastBorrowedSubject);
    }
}

// ================= AI RECOMMENDATION =================
void recommendBooks(const char *subject) {
    char sub[MAX_SUBJECT_LEN];
    strcpy(sub, subject);
    toLowerCase(sub);

    int found = 0;

    for (int i = 0; i < bookCount; i++) {
        if (!books[i].available) continue;

        char temp[MAX_SUBJECT_LEN];
        strcpy(temp, books[i].subject);
        toLowerCase(temp);

        if (strstr(temp, sub) || strstr(sub, temp)) {
            printf("- %s\n", books[i].title);
            found = 1;
        } else {
            int dist = levenshtein(sub, temp);
            if (dist <= strlen(sub)/2) {
                printf("- %s (similar)\n", books[i].title);
                found = 1;
            }
        }
    }

    if (!found) printf("No recommendations found.\n");
}