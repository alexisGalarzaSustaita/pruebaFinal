#include "main.h"


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

void eliminateSpacesAndPipes(Node *head) {
    Node *current = head;

    while (current != NULL) {
        char *production = current->production;
        char *newProduction = (char *)malloc(strlen(production) + 1);
        int count1 = 0, count2 = 0;

        while (production[count1] != '\0') {
            if (production[count1] != ' ' && production[count1] != '|') {
                newProduction[count2++] = production[count1];
            }
            count1++;
        }

        newProduction[count2] = '\0';
        strcpy(current->production, newProduction);
        free(newProduction);

        current = current->next;
    }
}


void eliminateAndGroupRuleIdentifier(Node *head) {
    Node *current = head;

    while (current != NULL) {
        char *production = current->production;
        char *newProduction = (char *)malloc(strlen(production) + 1);
        int count1 = 0, count2 = 0;
        int lastOccurrence = 0;

        while (production[count1] != '\0') {
            if (production[count1] == current->ruleIdentifier[0]) {
                if (!lastOccurrence && count1 > 0) {
                    newProduction[count2++] = '(';
                    newProduction[count2++] = production[count1 - 1];
                    newProduction[count2++] = '|';
                    newProduction[count2++] = ')';
                }
                count1++;
            } else {
                newProduction[count2++] = production[count1++];
            }
            lastOccurrence = 1;
        }

        newProduction[count2++] = current->ruleIdentifier[0];
        newProduction[count2] = '\0';

        strcpy(current->production, newProduction);
        free(newProduction);

        current = current->next;
    }
}


void keys(Node *head) {
    Node *current = head;

    while (current != NULL) {
        char *production = current->production;
        char *newProduction = (char *)malloc(strlen(production) + 1);
        int count1 = 0, count2 = 0;
        int firstNT = 0;
        int lastNT = 0;

        while (production[count1] != '\0') {
            if (isalpha(production[count1])) {
                if (!firstNT) {
                    firstNT = count1;
                }
                lastNT = count1;
            } else if (firstNT && lastNT != firstNT) {
                newProduction[count2++] = '{';
                strncpy(newProduction + count2, production + firstNT, lastNT - firstNT + 1);
                count2 += lastNT - firstNT + 1;
                newProduction[count2++] = '}';
                firstNT = 0;
                lastNT = 0;
            } else {
                newProduction[count2++] = production[count1];
            }
            count1++;
        }

        if (firstNT && lastNT != firstNT) {
            newProduction[count2++] = '{';
            strncpy(newProduction + count2, production + firstNT, lastNT - firstNT + 1);
            count2 += lastNT - firstNT + 1;
            newProduction[count2++] = '}';
        }

        newProduction[count2] = '\0';
        strcpy(current->production, newProduction);
        free(newProduction);

        current = current->next;
    }
}


void agregarProducciones(Node* head) {
    char producciones[100][100];
    int numProducciones = 0;

    Node* current = head;
    while (current != NULL) {
        strcpy(producciones[numProducciones], current->production);
        numProducciones++;
        current = current->next;
    }

    current = head;
    while (current != NULL) {
        char nuevaProduccion[100];
        strcpy(nuevaProduccion, current->production);

        char *token, *saveptr1;
        token = strtok_r(nuevaProduccion, " ", &saveptr1);
        while (token != NULL) {
            if (isalpha(*token) && strstr(nuevaProduccion, token) == token) {
                for (int i = 0; i < numProducciones; i++) {
                    if (strcmp(token, producciones[i]) == 0) {
                        strcat(nuevaProduccion, producciones[i]);
                        break;
                    }
                }
            }
            token = strtok_r(NULL, " ", &saveptr1);
        }

        strcpy(current->production, nuevaProduccion);
        current = current->next;
    }
}