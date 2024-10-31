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
    //addBracesToNonTerminals(head);
    //integrateProductions(head);
    // Output the contents of the linked list
    printList(head);

    // Free the linked list
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
    Node *next;

    while (current != NULL) {
        next = current->next;
        free(current->ruleIdentifier);
        free(current->production);
        free(current);
        current = next;
    }
}

// Function to create a linked list from the file
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

// Function to print the linked list
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



// Corrige eliminar espacios y tuberías solo cuando sea necesario
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

// Agrupa reglas con identificadores similares usando paréntesis de forma condicional
void eliminateAndGroupRuleIdentifier(Node *head) {
    Node *current = head;
    while (current != NULL) {
        char *production = current->production;
        char *newProduction = (char *)malloc(strlen(production) * 2);
        int i = 0, j = 0;

        // Skip the ruleIdentifier character
        i++;

        while (production[i] != '\0') {
            if (production[i] == '|') {
                // Copy the '|' directly
                newProduction[j++] = production[i++];
            } else {
                // Group the characters until the next '|' or end of string
                newProduction[j++] = '(';
                while (production[i] != '\0' && production[i] != '|') {
                    newProduction[j++] = production[i++];
                }
                newProduction[j++] = ')';
            }
        }
        newProduction[j] = '\0';

        strcpy(current->production, newProduction);
        free(newProduction);
        current = current->next;
    }
}
// Agrega llaves solo alrededor de no terminales válidos
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

// Función recursiva para reemplazar las ocurrencias de ruleIdentifiers
void replaceIdentifiers(Node *head, char **production, const char *ruleIdentifier) {
    for (Node *temp = head; temp != NULL; temp = temp->next) {
        // Solo reemplazamos si no es el mismo ruleIdentifier
        if (strcmp(temp->ruleIdentifier, ruleIdentifier) != 0) {
            char *pos = strstr(*production, temp->ruleIdentifier);
            while (pos != NULL) {
                // Construimos la nueva producción con el reemplazo
                size_t lenBefore = pos - *production;
                size_t lenAfter = strlen(pos + strlen(temp->ruleIdentifier));

                // Calculamos el nuevo tamaño para la producción
                size_t newSize = lenBefore + strlen(temp->production) + lenAfter + 1;

                // Creamos un nuevo buffer para la producción
                char *newProduction = (char *)malloc(newSize);

                // Copia antes del identificador
                strncpy(newProduction, *production, lenBefore);
                newProduction[lenBefore] = '\0';

                // Añade la producción correspondiente
                strcat(newProduction, temp->production);

                // Copia después del identificador
                strcat(newProduction, pos + strlen(temp->ruleIdentifier));

                // Liberamos la producción anterior
                free(*production);
                *production = newProduction; // Actualizamos el puntero a la nueva producción

                // Busca la próxima ocurrencia
                pos = strstr(*production, temp->ruleIdentifier);
            }
        }
    }
}

// Modificación de integrateProductions para usar la nueva función de reemplazo
void integrateProductions(Node *head) {
    Node *current = head;
    while (current != NULL) {
        // Hacemos una copia de la producción original
        char *production = strdup(current->production); 

        // Reemplazamos las ocurrencias de otros ruleIdentifiers
        replaceIdentifiers(head, &production, current->ruleIdentifier);

        // Reemplaza la producción original con la nueva producción integrada
        free(current->production); // Libera la memoria de la producción original
        current->production = production; // Asigna la nueva producción

        current = current->next;
    }
}