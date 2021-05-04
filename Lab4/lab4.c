#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>

#define BUF_SIZE 100
#define EXIT_SYMBOL '.'
#define LINE_END_SYMBOL '\n'

#define SUCCESS_STATUS 0
#define MEMORY_ALLOCATION_ERROR 1
#define EOF_STATUS 2

typedef struct ListNode ListNode;

struct ListNode {
    char *value;
    ListNode *next;
    ListNode *prev;
};

ListNode* initList() {
    ListNode *head = (ListNode*) malloc(sizeof(ListNode));
    if (head == NULL) {
        perror("Error on head node creation");
        return NULL;
    }
    head->value = NULL;
    head->next = NULL;
    head->prev = NULL;
    return head;
}

int addNode(ListNode **head, char *value, int valueLen) {
    ListNode *node = (ListNode*) malloc(sizeof(ListNode));
    if (node == NULL) {
        perror("Error on node creation");
        return MEMORY_ALLOCATION_ERROR;
    }
    node->value = (char*) malloc(sizeof(char) * valueLen);
    if (node->value == NULL) {
        perror("Error on line copy creation");
        return MEMORY_ALLOCATION_ERROR;
    }
    strncpy(node->value, value, valueLen);
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
    char *tmpLine, *fgetsRes;
    char endSymbol;
    *line = NULL;
    *lineLen = 0;
    do {
        // Выделение / перевыделение памяти
        tmpLine = (char*) realloc(*line, sizeof(char) * (*lineLen + BUF_SIZE));
        if (tmpLine == NULL) {
            perror("Error on line realloc");
            return MEMORY_ALLOCATION_ERROR;
        }
        *line = tmpLine;
        // Чтение строки из входного потока
        fgetsRes = fgets(&(*line)[*lineLen], BUF_SIZE, stdin);
        if (fgetsRes == NULL) {
            printf("Unexpected file end");
            return EOF_STATUS;
        }
        // Запись в переменную размера строки
        *lineLen = (int)strnlen(*line, *lineLen + BUF_SIZE);
        // Запись в переменную последнего символа строки
        endSymbol = (*line)[(*lineLen) - 1];

    } while (endSymbol != LINE_END_SYMBOL);

    return SUCCESS_STATUS;
}

int main() {
    // Объявление указателя на строку, в которую будут
    // записываться данные из входного потока
    char *line;
    int lineLen;

    // Создание указателей на начало и конец списка,
    // чтобы можно было обойти список в обратном порядке
    ListNode *head = initList();
    if (head == NULL) {
        return MEMORY_ALLOCATION_ERROR;
    }
    ListNode *tail = head;

    // Построчное чтение из входного потока, пока не дойдём до символа '.'
    while (true) {
        int readLineStatus = readLine(&line, &lineLen);
        if (readLineStatus != SUCCESS_STATUS) {
            freeList(head);
            free(line);
            return readLineStatus;
        }
        if (line[0] == EXIT_SYMBOL) {
            break;
        }
        int addNodeStatus = addNode(&head, line, lineLen);
        if (addNodeStatus != SUCCESS_STATUS) {
            freeList(head);
            free(line);
            return addNodeStatus;
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
