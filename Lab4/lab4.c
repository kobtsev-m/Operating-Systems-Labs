#include <stdio.h>
#include <malloc.h>
#include <string.h>

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
        perror("Memory allocation error on head node creation");
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
        perror("Memory allocation error on node creation");
        return MEMORY_ALLOCATION_ERROR;
    }
    node->value = (char*) malloc(sizeof(char) * valueLen);
    if (node->value == NULL) {
        perror("Memory allocation error on line copy creation");
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
    char *fgetsRes = fgets(*line, BUF_SIZE, stdin);
    char *tmpLine;
    int k = 2;
    *lineLen = (int)strnlen(*line, BUF_SIZE);
    while ((*line)[(*lineLen) - 1] != LINE_END_SYMBOL && fgetsRes != NULL) {
        tmpLine = realloc(*line, sizeof(char) * BUF_SIZE*k);
        if (tmpLine == NULL) {
            perror("Memory reallocation error on line oversize");
            return MEMORY_ALLOCATION_ERROR;
        }
        *line = tmpLine;
        fgetsRes = fgets(&(*line)[*lineLen], BUF_SIZE*k - (*lineLen), stdin);
        *lineLen = (int)strnlen(*line, BUF_SIZE*k);
        k *= 2;
    }
    if (fgetsRes == NULL) {
        return EOF_STATUS;
    }
    return SUCCESS_STATUS;
}

int main() {
    // Объявление указателя на строку, в которую будут
    // записываться данные из входного потока
    char *line = (char*) malloc(sizeof(char) * BUF_SIZE);
    if (line == NULL) {
        perror("Memory allocation error on line init");
        return MEMORY_ALLOCATION_ERROR;
    }
    int lineLen = 0;

    // Создание указателей на начало и конец списка,
    // чтобы можно было обойти список в обратном порядке
    ListNode *head = initList();
    if (head == NULL) {
        free(line);
        return MEMORY_ALLOCATION_ERROR;
    }
    ListNode *tail = head;

    // Построчное чтение из входного потока, пока не дойдём до символа '.'
    while (1) {
        int readLineStatus = readLine(&line, &lineLen);
        if (readLineStatus == MEMORY_ALLOCATION_ERROR) {
            freeList(head);
            free(line);
            return MEMORY_ALLOCATION_ERROR;
        }
        if (line[0] == EXIT_SYMBOL || readLineStatus == EOF_STATUS) {
            break;
        }
        int addNodeStatus = addNode(&head, line, lineLen);
        if (addNodeStatus == MEMORY_ALLOCATION_ERROR) {
            freeList(head);
            free(line);
            return MEMORY_ALLOCATION_ERROR;
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
