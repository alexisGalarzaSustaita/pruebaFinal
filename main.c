#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 100

typedef struct Node {
    char *ruleIdentifier;
    char *production;
    struct Node *next;
} Node;

// Function prototypes
Node *createNode(const char *ruleIdentifier, const char *production);
void appendNode(Node **head, const char *ruleIdentifier, const char *production);
void freeLinkedList(Node *head);
Node *createLinkedList(FILE *file);
void printList(Node *head);
void splitLine(const char *line, char *ruleIdentifier, char *production);
Node *findNode(Node *head, const char *ruleIdentifier);
void appendProduction(Node *node, const char *production);
void appendOrUpdateNode(Node **head, const char *ruleIdentifier, const char *production);
void eliminaEspaciosYpipes(Node *head);
void agruparProducciones(Node *head);
void reemplazarConcurrencias(Node *head);

int main() {
    FILE *grammar = fopen("gramatica2.txt", "r");
    if (grammar == NULL) {
        perror("Error opening the file");
        return 1;
    }

    Node *head = createLinkedList(grammar);
    fclose(grammar);

    // Procesar las producciones en orden
    eliminaEspaciosYpipes(head);
    agruparProducciones(head);
    reemplazarConcurrencias(head);

    // Imprimir la lista enlazada después del procesamiento
    printList(head);

    // Liberar la lista enlazada
    freeLinkedList(head);

    return 0;
}

// Function to create a new node
Node *createNode(const char *ruleIdentifier, const char *production) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->ruleIdentifier = strdup(ruleIdentifier);
    newNode->production = strdup(production);
    newNode->next = NULL;
    return newNode;
}

// Function to append a node to the list
void appendNode(Node **head, const char *ruleIdentifier, const char *production) {
    Node *newNode = createNode(ruleIdentifier, production);
    if (*head == NULL) {
        *head = newNode;
    } else {
        Node *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

// Function to free the linked list
void freeLinkedList(Node *head) {
    Node *current = head;
    Node *nextNode;

    while (current != NULL) {
        nextNode = current->next;
        free(current->ruleIdentifier);
        free(current->production);
        free(current);
        current = nextNode;
    }
}

// Function to create a linked list from the file
Node* createLinkedList(FILE *file) {
    Node *head = NULL; 
    char line[MAX_LINE_LENGTH];
    char ruleIdentifier[MAX_LINE_LENGTH];
    char production[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        splitLine(line, ruleIdentifier, production);
        appendOrUpdateNode(&head, ruleIdentifier, production);
    }

    return head;
}

// Function to print the linked list
void printList(Node *head) {
    Node *current = head;
    while (current != NULL) {
        printf("%s -> %s\n", current->ruleIdentifier, current->production);
        current = current->next;
    }
}

void splitLine(const char *line, char *ruleIdentifier, char *production) {
    const char *delimiter = strstr(line, "->");
    if (delimiter != NULL) {
        strncpy(ruleIdentifier, line, delimiter - line);
        ruleIdentifier[delimiter - line] = '\0';
        strcpy(production, delimiter + 2);
    }
}

Node* findNode(Node *head, const char *ruleIdentifier) {
    Node *current = head;
    while (current != NULL) {
        if (strcmp(current->ruleIdentifier, ruleIdentifier) == 0) {
            return current; 
        }
        current = current->next; 
    } 
    return NULL; 
}

void appendProduction(Node *node, const char *production) {
    size_t newSize = strlen(node->production) + strlen(production) + 4;
    node->production = (char *)realloc(node->production, newSize); 
    strcat(node->production, " | ");
    strcat(node->production, production);
}

void appendOrUpdateNode(Node **head, const char *ruleIdentifier, const char *production) {
    Node *existingNode = findNode(*head, ruleIdentifier);
    if (existingNode != NULL) {
        appendProduction(existingNode, production);
    } else {
        appendNode(head, ruleIdentifier, production);
    }
}

// Function to eliminate spaces and pipes
void eliminaEspaciosYpipes(Node *head) {
    Node *current = head;
    while (current != NULL) {
        int len = strlen(current->production);
        char *line = (char *)malloc(len + 1);
        int count1 = 0, count2 = 0;

        while (count1 < len) {
            if (current->production[count1] != ' ' && current->production[count1] != '|') {
                line[count2++] = current->production[count1];
            }
            count1++;
        }
        line[count2] = '\0';

        free(current->production);
        current->production = line;
        current = current->next;
    }
}

// Function to group productions
void agruparProducciones(Node *head) {
    Node *current = head;

    while (current != NULL) {
        char updatedProduction[MAX_LINE_LENGTH] = "";
        char *production = current->production;
        size_t ruleLen = strlen(current->ruleIdentifier);

        int count1 = 0;
        while (count1 < strlen(production)) {
            if (strncmp(&production[count1], current->ruleIdentifier, ruleLen) == 0) {
                if (count1 > 0) {
                    strncat(updatedProduction, "(", 1);
                    strncat(updatedProduction, &production[count1 - 1], 1);
                    strncat(updatedProduction, ")", 1);
                }
                strcat(updatedProduction, "{");
                strncat(updatedProduction, current->ruleIdentifier, ruleLen);
                strcat(updatedProduction, "}");
                count1 += ruleLen;
            } else {
                strncat(updatedProduction, &production[count1], 1);
                count1++;
            }
        }
        free(current->production);
        current->production = strdup(updatedProduction);
        current = current->next;
    }
}

// Function to replace occurrences of other ruleIdentifiers in productions
void reemplazarConcurrencias(Node *head) {
    Node *current = head;

    while (current != NULL) {
        char updatedProduction[MAX_LINE_LENGTH] = "";
        char *production = current->production;
        for (int i = 0; i < strlen(production); i++) {
            if (production[i] == '{') {
                // Ignore the opening brace
                while (i < strlen(production) && production[i] != '}') {
                    i++;
                }
                if (i < strlen(production) && production[i] == '}') {
                    // Ignore the closing brace
                    continue;
                }
            }
            strncat(updatedProduction, &production[i], 1);
        }
        free(current->production);
        current->production = strdup(updatedProduction);
        current = current->next;
    }
}