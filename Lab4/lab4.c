#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>

#define SUCCESS_STATUS (0)
#define MEMORY_ALLOCATION_ERROR (1)
#define MEMORY_REALLOCATION_ERROR (2)
#define EOF_STATUS (3)

#define LINE_END_SYMBOL ('\n')
#define TERMINAL_ZERO ('\0')
#define EXIT_SYMBOL ('.')
#define BUF_SIZE (100)

typedef struct ListNode {
    char *value;
    struct ListNode *next;
    struct ListNode *prev;
} ListNode;

int initList(ListNode **head, ListNode **tail) {
    *head = (ListNode*) malloc(sizeof(ListNode));
    if (*head == NULL) {
        perror("Error on head node creation");
        return MEMORY_ALLOCATION_ERROR;
    }
    (*head)->value = NULL;
    (*head)->next = NULL;
    (*head)->prev = NULL;
    *tail = *head;
    return SUCCESS_STATUS;
}

int addNode(ListNode **head, char *value, int valueLen) {
    ListNode *node = (ListNode*) malloc(sizeof(ListNode));
    if (node == NULL) {
        perror("Error on node creation");
        return MEMORY_ALLOCATION_ERROR;
    }
    node->value = (char*) malloc(sizeof(char) * (valueLen + 1));
    if (node->value == NULL) {
        perror("Error on line copy creation");
        return MEMORY_ALLOCATION_ERROR;
    }
    // Копирование значения из введённой строки
    strncpy(node->value, value, valueLen);
    node->value[valueLen] = TERMINAL_ZERO;
    // Переопределение указателей
    node->next = *head;
    node->prev = NULL;
    (*head)->prev = node;
    *head = node;
    return SUCCESS_STATUS;
}

void freeList(ListNode *head) {
    ListNode *tmpNode;
    while (head != NULL) {
        tmpNode = head;
        head = head->next;
        free(tmpNode);
    }
}

int readLine(char **line, int *lineLen) {
    int idx = 0;
    while (true) {
        // Чтение строки из входного потока
        char *fgetsRes = fgets(&(*line)[idx], BUF_SIZE, stdin);
        if (fgetsRes == NULL) {
            fprintf(stderr, "Unexpected file end\n");
            return EOF_STATUS;
        }
        // Запись в переменную текущего размера строки
        idx = (int) strnlen(*line, idx + BUF_SIZE);
        // Проверка на конец строки
        if ((*line)[idx - 1] == LINE_END_SYMBOL) {
            break;
        }
        // Перевыделение памяти под строку
        char *tmp = (char*) realloc(*line, sizeof(char) * (idx + BUF_SIZE));
        if (tmp == NULL) {
            perror("Error on line realloc");
            return MEMORY_REALLOCATION_ERROR;
        }
        *line = tmp;
    }
    *lineLen = idx;
    return SUCCESS_STATUS;
}

int main() {
    // Выделение пямяти под строку, в которую будут
    // записываться данные из входного потока
    char *line = (char*) malloc(sizeof(char) * BUF_SIZE);
    if (line == NULL) {
        perror("Error on line initialization");
        return MEMORY_ALLOCATION_ERROR;
    }
    int lineLen;

    // Создание указателей на начало и конец списка
    ListNode *head, *tail;
    int initListRes = initList(&head, &tail);
    if (initListRes != SUCCESS_STATUS) {
        free(line);
        return initListRes;
    }

    // Построчное чтение из входного потока, пока не дойдём до символа '.'
    while (true) {
        int readLineRes = readLine(&line, &lineLen);
        if (readLineRes != SUCCESS_STATUS) {
            freeList(head);
            free(line);
            return readLineRes;
        }
        if (line[0] == EXIT_SYMBOL) {
            break;
        }
        int addNodeRes = addNode(&head, line, lineLen);
        if (addNodeRes != SUCCESS_STATUS) {
            freeList(head);
            free(line);
            return addNodeRes;
        }
    }

    // Вывод списка
    for (ListNode *node = tail->prev; node != NULL; node = node->prev) {
        printf("%s", node->value);
    }

    // Очистка памяти
    freeList(head);
    free(line);

    return SUCCESS_STATUS;
}
