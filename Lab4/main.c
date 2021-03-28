#include <stdio.h>
#include <malloc.h>
#include <string.h>

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
    node->value = (char*) malloc(sizeof(char) * (strlen(value)+1));
    strcpy(node->value, value);
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
    char *line = (char*) malloc(sizeof(char) * BUFSIZ);

    ListNode *head = initList();
    ListNode *tail = head;

    while (1) {
        char *res = fgets(line, sizeof(line), stdin);
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
