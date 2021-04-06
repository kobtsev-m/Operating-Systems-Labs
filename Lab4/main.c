#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <limits.h>

typedef struct ListNode ListNode;

struct ListNode {
    char *value;
    ListNode *next;
    ListNode *prev;
};

ListNode* initList() {
    ListNode *head = (ListNode*) malloc(sizeof(ListNode));
    head->value = NULL;
    head->next = NULL;
    head->prev = NULL;
    return head;
}

void addNode(ListNode **head, char *value) {
    ListNode *node = (ListNode*) malloc(sizeof(ListNode));
    int stringLength = strnlen(value, LINE_MAX) + 1;
    node->value = (char*) malloc(sizeof(char) * stringLength);
    strncpy(node->value, value, stringLength);
    node->next = *head;
    node->prev = NULL;
    (*head)->prev = node;
    *head = node;
}

void freeList(ListNode *head) {
    ListNode *tmp;
    while (head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp);
    }
}

int main() {
    char *line = (char*) malloc(sizeof(char) * LINE_MAX);

    ListNode *head = initList();
    ListNode *tail = head;

    while (1) {
        char *res = fgets(line, LINE_MAX, stdin);
        if (line[0] == '.' || res == NULL) {
            break;
        }
        addNode(&head, line);
    }

    for (ListNode *node = tail->prev; node != NULL; node = node->prev) {
        printf("%s", node->value);
    }

    freeList(head);
    free(line);

    return 0;
}
