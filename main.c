#include "main.h"

int main() {
    FILE *grammar = fopen("gramatica2.txt", "r");

    if (grammar == NULL) {
        perror("Error opening the file");
        return 1;
    }

    Node *head = createLinkedList(grammar);
    
    fclose(grammar);

    eliminateSpacesAndPipes(head);
    eliminateAndGroupRuleIdentifier(head);
    addBracesToNonTerminals(head);
    integrateProductions(head);
   
    printList(head);

   
    freeLinkedList(head);    

    return 0;
}


Node *createNode(const char *ruleIdentifier, const char *production) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->ruleIdentifier = strdup(ruleIdentifier);
    newNode->production = strdup(production);
    newNode->next = NULL;
    return newNode;
}


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


void freeLinkedList(Node *head) {
    Node *current = head;
    Node *next;

    while (current != NULL) {
        next = current->next;
        free(current->ruleIdentifier);
        free(current->production);
        free(current);
        current = next;
    }
}


Node* createLinkedList(FILE *file) {
    Node *head = NULL; 
    char line[MAX_LINE_LENGTH];
    char ruleIdentifier[MAX_LINE_LENGTH];
    char production[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), file)) {
        // Remove the newline character if present
        line[strcspn(line, "\n")] = '\0';
        splitLine(line, ruleIdentifier, production);
        appendOrUpdateNode(&head, ruleIdentifier, production);
    }

    return head;
}


void printList(Node *head) {
    Node *current = head;

    while (current != NULL) {
        printf("%s -> %s\n", current->ruleIdentifier, current->production);
        current = current->next;
    }
}

void splitLine(const char *line, char *ruleIdentifier, char *production){
    const char *delimiter = strstr(line, "->");
    if (delimiter != NULL) {
        strncpy(ruleIdentifier, line, delimiter - line);
        ruleIdentifier[delimiter - line] = '\0';
        strcpy(production, delimiter + 2);
    }
}


Node* findNode(Node *head, const char *ruleIdentifier){
    Node *current = head;
    while (current != NULL) {
        if (strcmp(current->ruleIdentifier, ruleIdentifier) == 0) {
            return current; 
        }
        current = current->next; 
    } 
    return NULL; 
}

void appendProduction(Node *node, const char *production){
    size_t newSize = strlen(node->production) + strlen(production) + 4;
    node->production = (char *)realloc(node->production, newSize); 
    strcat(node->production, " | ");
    strcat(node->production, production);
}

void appendOrUpdateNode(Node **head, const char *ruleIdentifier, const char *production){
    Node *existingNode = findNode(*head, ruleIdentifier);
    if (existingNode != NULL) {
        appendProduction(existingNode, production);
    } else {
        Node *newNode = createNode(ruleIdentifier, production);
        if (*head == NULL){
            *head = newNode; 
        } else {
            Node *temp = *head; 
            while (temp->next != NULL) {
                temp = temp->next;
            }
            temp->next = newNode; 
        }
    }
}




void eliminateSpacesAndPipes(Node *head) {
    Node *current = head;
    while (current != NULL) {
        char *production = current->production;
        char *newProduction = (char *)malloc(strlen(production) + 1);
        int count1 = 0, count2 = 0;

        while (production[count1] != '\0') {
            if (!(production[count1] == ' ' && (count1 == 0 || production[count1 - 1] == ' ' || production[count1 + 1] == ' '))) {
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
        char *newProduction = (char *)malloc(strlen(production) * 2);
        int i = 0, j = 0;
        while (production[i] != '\0') {
            if (production[i] == current->ruleIdentifier[0]) {
                newProduction[j++] = '(';
                newProduction[j++] = production[i];
                newProduction[j++] = '|';
                i++;
                while (production[i] != '\0' && production[i] != '|') {
                    newProduction[j++] = production[i++];
                }
                newProduction[j++] = ')';
            } else {
                newProduction[j++] = production[i++];
            }
        }
        newProduction[j] = '\0';
        strcpy(current->production, newProduction);
        free(newProduction);
        current = current->next;
    }
}


void addBracesToNonTerminals(Node *head) {
    Node *current = head;
    while (current != NULL) {
        char *production = current->production;
        char *newProduction = (char *)malloc(strlen(production) * 2);
        int i = 0, j = 0;
        
        while (production[i] != '\0') {
            if (isalpha(production[i])) {
                newProduction[j++] = '{';
                while (isalpha(production[i])) {
                    newProduction[j++] = production[i++];
                }
                newProduction[j++] = '}';
            } else {
                newProduction[j++] = production[i++];
            }
        }

        newProduction[j] = '\0';
        strcpy(current->production, newProduction);
        free(newProduction);
        current = current->next;
    }
}


void integrateProductionsRecursive(Node *current, Node *head) {
    if (current == NULL) {
        return;
    }

    char *production = current->production;
    char integratedProduction[1000];
    strcpy(integratedProduction, production);

    Node *temp = head;
    while (temp != NULL) {
        if (temp != current && strstr(production, temp->ruleIdentifier) != NULL) {
            
            char *insertionPoint = strstr(integratedProduction, temp->ruleIdentifier);

            
            strcat(insertionPoint, " | ");
            strcat(insertionPoint, temp->production);
        }
        temp = temp->next;
    }

    strcpy(current->production, integratedProduction);
    integrateProductionsRecursive(current->next, head);
}

void integrateProductions(Node *head) {
    integrateProductionsRecursive(head, head);
}