#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Определение структуры узла списка
struct Node {
    char *str;
    struct Node *next;
};

// Функция для добавления новой строки в список
void append(struct Node **head, const char *input) {
    struct Node *new_node = (struct Node *)malloc(sizeof(struct Node)); // Выделение памяти под новый узел
    if (new_node == NULL) {
        perror("Unable to allocate memory for new node");
        exit(1);
    }

    // Копируем введенную строку в новый узел
    new_node->str = (char *)malloc(strlen(input) + 1); // Выделяем память для строки
    if (new_node->str == NULL) {
        perror("Unable to allocate memory for string");
        exit(1);
    }
    strcpy(new_node->str, input); // Копируем строку

    new_node->next = NULL;

    // Если список пуст, новый узел становится головой списка
    if (*head == NULL) {
        *head = new_node;
    } else {
        // Иначе добавляем узел в конец списка
        struct Node *temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_node;
    }
}

// Функция для вывода всех строк из списка
void print_list(struct Node *head) {
    struct Node *temp = head;
    while (temp != NULL) {
        printf("%s", temp->str); // Строки уже содержат символ новой строки из fgets
        temp = temp->next;
    }
}

// Функция для освобождения памяти, выделенной под список
void free_list(struct Node *head) {
    struct Node *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp->str); // Освобождаем память под строку
        free(temp);      // Освобождаем память под узел
    }
}

int main() {
    struct Node *head = NULL; // Указатель на голову списка
    char input[1024];         // Массив для хранения введенной строки


    while (true) {
        // Чтение строки
        if (fgets(input, sizeof(input), stdin) == NULL) {
            perror("Error reading input");
            exit(1);
        }

        if (input[0] == '.') {
            break;
        }

        // Вставляем строку в список
        append(&head, input);
    }

    // Вывод всех строк из списка

    print_list(head);

    // Освобождаем память
    free_list(head);

    return 0;
}
