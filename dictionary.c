#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAIN_ENTITY_POINTER 0
#define EMPTY_POINTER -1
#define DATA_BLOCK_SIZE 50

//ESTRUCTURAS
typedef struct Entity{
    char name[DATA_BLOCK_SIZE];
    long dataPointer;
    long attributesPointer;
    long nextEntity;
} ENTITY;

typedef struct Attribute {
    char name[DATA_BLOCK_SIZE];
    bool isPrimary;
    long type;
    long size;
    long nextAttribute;
} ATTRIBUTE;

//PROTOTIPOS
FILE* initializeDataDictionary(const char *dictionaryName);
int appendEntity(FILE* dataDicictionary, ENTITY newEntity);
void reorderEntities(FILE* dataDictionary, long currentEntityPointer, const char* newEntityName, long newEntityDirection);
void createEntity(FILE* dataDictionary);
ENTITY removeEntity(FILE* dataDictionary, long currentEntityPointer, const char* entityName);
void captureEntities(FILE* dataDictionary);
void showEntities(FILE* dataDictionary);
void searchEntityByName(FILE* dataDictionary, const char* entityName, ENTITY* resultEntity);
void askNameEntity(FILE* dataDictionary, ENTITY* resultEntity);


//ATTRIBUTES
int appendAttribute(FILE* dataDictionary, ATTRIBUTE newAttribute);
void reorderAttributes(FILE* dataDictionary, long currentAttributePointer, const char* newAttributeName, long newAttributeDirection);
void attributeSize(ATTRIBUTE newAtribute);
void createAttribute(FILE* dataDictionary, ENTITY currentEntity);
ATTRIBUTE removeAttribute(FILE* dataDictionary, long currentAttributePointer, const char* attributeName);
void captureAttributes(FILE* dataDictionary, ENTITY currentEntity);
void captureAttributesForEntity(FILE* dataDictionary);

int main(){

    FILE* dictionary = initializeDataDictionary("datos.dat");

    captureEntities(dictionary);
    showEntities(dictionary);
    captureAttributesForEntity(dictionary);
    showEntities(dictionary);
    fclose(dictionary);

    return 0;
}


//FUNCIONES
FILE* initializeDataDictionary(const char *dictionaryName) {
    long mainHeader = EMPTY_POINTER;

    printf("Initializing Data Dictionary...\n");

    FILE *dictionary = fopen(dictionaryName, "w+");

    fwrite(&mainHeader, sizeof(mainHeader), 1, dictionary);

    return dictionary;
}

//Agrega una nueva entidad
int appendEntity(FILE* dataDictionary, ENTITY newEntity) {

    fseek(dataDictionary, 0, SEEK_END);

    long entityDirection = ftell(dataDictionary);

    fwrite(newEntity.name, DATA_BLOCK_SIZE, 1, dataDictionary);
    fwrite(&newEntity.dataPointer, sizeof(long), 1, dataDictionary);
    fwrite(&newEntity.attributesPointer, sizeof(long), 1, dataDictionary);
    fwrite(&newEntity.nextEntity, sizeof(long), 1, dataDictionary);

    return entityDirection;
}
//Ordena las entidades
void reorderEntities(FILE* dataDictionary, long currentEntityPointer, const char* newEntityName, long newEntityDirection){
    long currentEntityDirection = -1;

    fseek(dataDictionary, currentEntityPointer, SEEK_SET);
    fread(&currentEntityDirection, sizeof(currentEntityDirection), 1, dataDictionary);

    if (currentEntityDirection == -1) {

        fseek(dataDictionary, currentEntityPointer, SEEK_SET);
        fwrite(&newEntityDirection, sizeof(long), 1, dataDictionary);
    }
    else {
        char currentEntityName[DATA_BLOCK_SIZE];
        long nextEtityDirection;
        long nextHeaderPointer;


        fseek(dataDictionary, currentEntityDirection, SEEK_SET);

        fread(&currentEntityName, sizeof(char), DATA_BLOCK_SIZE, dataDictionary);
        nextHeaderPointer = ftell(dataDictionary) + (sizeof(long) * 2);

        if (strcmp(currentEntityName, newEntityName) < 0) {

            reorderEntities(dataDictionary, nextHeaderPointer, newEntityName, newEntityDirection);
        }
        else {

            fseek(dataDictionary, currentEntityPointer, SEEK_SET);
            fwrite(&newEntityDirection, sizeof(long), 1, dataDictionary);

            fseek(dataDictionary, newEntityDirection + DATA_BLOCK_SIZE + (sizeof(long) * 2), SEEK_SET);
            fwrite(&currentEntityDirection, sizeof(long), 1, dataDictionary);
        }
    }
}
//Crea una nueva entidad
void createEntity(FILE* dataDictionary) {
    ENTITY newEntity;

    fflush(stdin);
    printf("\nEnter the Entity name: ");
    fgets(newEntity.name, sizeof(newEntity.name), stdin);
    newEntity.name[strcspn(newEntity.name, "\n")] = '\0';
    newEntity.dataPointer = EMPTY_POINTER;
    newEntity.attributesPointer = EMPTY_POINTER;
    newEntity.nextEntity = EMPTY_POINTER;

    long entityDirection = appendEntity(dataDictionary, newEntity);
    reorderEntities(dataDictionary, MAIN_ENTITY_POINTER, newEntity.name, entityDirection);
}
//Eliminacion logica de una entidad
ENTITY removeEntity(FILE* dataDictionary, long currentEntityPointer, const char* entityName){
    long currentEntityDirection = -1;

    fseek(dataDictionary, currentEntityDirection, SEEK_SET);
    fread(&currentEntityDirection, sizeof(long), 1, dataDictionary);

    if(currentEntityDirection == -1){
        ENTITY emptyEntity = {0};
        return emptyEntity;
    }
    else{
        ENTITY resultEntity;
        long nextEntityDirection;
        long nextHeaderPointer;

        fseek(dataDictionary, currentEntityDirection, SEEK_SET);
        fread(resultEntity.name, sizeof(char), DATA_BLOCK_SIZE, dataDictionary);
        nextHeaderPointer = ftell(dataDictionary) + (sizeof(long) * 2);

        if(strcmp(resultEntity.name, entityName) == 0){
            fread(&resultEntity.dataPointer, sizeof(long), 1, dataDictionary);
            fread(&resultEntity.attributesPointer, sizeof(long), 1, dataDictionary);
            fread(&resultEntity.nextEntity, sizeof(long), 1, dataDictionary);

            fseek(dataDictionary, currentEntityPointer, SEEK_SET);
            fwrite(&resultEntity.nextEntity, sizeof(long), 1, dataDictionary);

            return resultEntity;
        }
        else{
            return removeEntity(dataDictionary, nextHeaderPointer, entityName);
        }
    }
}

//Captura entidades
void captureEntities(FILE* dataDictionary) {
    int keepCapturing = 1;

    fflush(stdin);
    while (keepCapturing) {
        printf("\n--- Capturing Entity ---\n");
        createEntity(dataDictionary);

        printf("\nDo you want to add another entity? 1)Yes 0)No: ");
        scanf("%d", &keepCapturing);
        fflush(stdin);
    }
}

//Muestra los nombres de las entidades
void showEntities(FILE* dataDictionary) {
  long currentEntityDirection = -1;

  fseek(dataDictionary, MAIN_ENTITY_POINTER, SEEK_SET);
  fread(&currentEntityDirection, sizeof(currentEntityDirection), 1, dataDictionary);

  if (currentEntityDirection == EMPTY_POINTER) {
    printf("\nNo entities found.\n");
    return;
  }

  printf("\n--- Entities List ---\n");
  printf("NAME\t    METADATA\tATTRIBUTES POINTER\tNEXT ENTITY\t\n");
  while (currentEntityDirection != EMPTY_POINTER) {
    ENTITY currentEntity;

        fseek(dataDictionary, currentEntityDirection, SEEK_SET);
        fread(currentEntity.name, DATA_BLOCK_SIZE, 1, dataDictionary);
        fread(&currentEntity.dataPointer, sizeof(long), 1, dataDictionary);
        fread(&currentEntity.attributesPointer, sizeof(long), 1, dataDictionary);
        fread(&currentEntityDirection, sizeof(long), 1, dataDictionary);

    printf("\%s\t       %ld\t           %ld\t          %ld\t\n", currentEntity.name, currentEntity.dataPointer, currentEntity.attributesPointer, currentEntityDirection);
  }
}

// Buscar entidad por nombre: pasa un puntero a la entidad para modificarlo si se encuentra la entidad
void searchEntityByName(FILE* dataDictionary, const char* entityName, ENTITY* resultEntity) {
    long currentEntityDirection = -1;

    fseek(dataDictionary, MAIN_ENTITY_POINTER, SEEK_SET);
    fread(&currentEntityDirection, sizeof(currentEntityDirection), 1, dataDictionary);

    while (currentEntityDirection != EMPTY_POINTER) {
        ENTITY currentEntity;

        fseek(dataDictionary, currentEntityDirection, SEEK_SET);
        fread(currentEntity.name, DATA_BLOCK_SIZE, 1, dataDictionary);
        fread(&currentEntity.dataPointer, sizeof(long), 1, dataDictionary);
        fread(&currentEntity.attributesPointer, sizeof(long), 1, dataDictionary);
        fread(&currentEntity.nextEntity, sizeof(long), 1, dataDictionary);

        if (strcmp(currentEntity.name, entityName) == 0) {
            *resultEntity = currentEntity;
            return;
        }

        currentEntityDirection = currentEntity.nextEntity;
    }

    ENTITY emptyEntity = {0};
    *resultEntity = emptyEntity;
    printf("\nEntity '%s' not found.\n", entityName);
}

// Función para preguntar el nombre de una entidad y buscarla
void askNameEntity(FILE* dataDictionary, ENTITY* resultEntity) {
    char entityName[DATA_BLOCK_SIZE];

    printf("\nEnter the name of the entity: ");
    fgets(entityName, sizeof(entityName), stdin);
    entityName[strcspn(entityName, "\n")] = '\0';

    searchEntityByName(dataDictionary, entityName, resultEntity);

    if (resultEntity->name[0] == '\0') {
        printf("No entity found with name: '%s'\n", entityName);
    }
}

//ATTRIBUTES

//Aggrega el atributo a la entidad a la que pertenece
int appendAttribute(FILE* dataDictionary, ATTRIBUTE newAttribute) {

    fseek(dataDictionary, 0, SEEK_END);

    long attributeDirection = ftell(dataDictionary);

    fwrite(newAttribute.name, DATA_BLOCK_SIZE, 1, dataDictionary);
    fwrite(&newAttribute.isPrimary, sizeof(bool), 1, dataDictionary);
    fwrite(&newAttribute.type, sizeof(long), 1, dataDictionary);
    fwrite(&newAttribute.size, sizeof(long), 1, dataDictionary);
    fwrite(&newAttribute.nextAttribute, sizeof(long), 1, dataDictionary);

    return attributeDirection;
}
//Ordena los atributos de la entidad
void reorderAttributes(FILE* dataDictionary, long currentAttributePointer, const char* newAttributeName, long newAttributeDirection){
    long currentAttributeDirection = -1;

    fseek(dataDictionary, currentAttributePointer, SEEK_SET);
    fread(&currentAttributeDirection, sizeof(currentAttributeDirection), 1, dataDictionary);

    if (currentAttributeDirection == -1) {

        fseek(dataDictionary, currentAttributePointer, SEEK_SET);
        fwrite(&newAttributeDirection, sizeof(long), 1, dataDictionary);
    }
    else {
        char currentAttributeName[DATA_BLOCK_SIZE];
        long nextAttributeDirection;
        long nextHeaderPointer;


        fseek(dataDictionary, currentAttributeDirection, SEEK_SET);

        fread(&currentAttributeName, sizeof(char), DATA_BLOCK_SIZE, dataDictionary);
        nextHeaderPointer = ftell(dataDictionary) + sizeof(bool) + (sizeof(long) * 2);

        if (strcmp(currentAttributeName, newAttributeName) < 0) {

            reorderAttributes(dataDictionary, nextHeaderPointer, newAttributeName, newAttributeDirection);
        }
        else {

            fseek(dataDictionary, currentAttributePointer, SEEK_SET);
            fwrite(&newAttributeDirection, sizeof(long), 1, dataDictionary);

            fseek(dataDictionary, newAttributeDirection + DATA_BLOCK_SIZE + sizeof(bool) + (sizeof(long) * 2), SEEK_SET);
            fwrite(&currentAttributeDirection, sizeof(long), 1, dataDictionary);
        }
    }
}

//Asignar el tamaño del atributo
void attributeSize(ATTRIBUTE newAtribute){
    int number;

    switch (newAtribute.type){
        case 1:
            newAtribute.size = sizeof(int);
            break;

        case 2:
            newAtribute.size = sizeof(long);
            break;

        case 3:
            newAtribute.size = sizeof(float);
            break;

        case 4:
            printf("Enter string size: ");
            scanf("%d", &number);
            fflush(stdin);
            newAtribute.size = sizeof(char) * number;
            break;

        case 5:
            newAtribute.size = sizeof(bool);
            break;

        default:
            printf("Invalid attribute type.\n");
            break;
    }
}

void createAttribute(FILE* dataDictionary, ENTITY currentEntity) {
    ATTRIBUTE newAttribute;
    char response[10];

    fflush(stdin);
    printf("\nEnter the Attribute name: ");
    fgets(newAttribute.name, sizeof(newAttribute.name), stdin);
    newAttribute.name[strcspn(newAttribute.name, "\n")] = '\0';
    fflush(stdin);

    printf("\nIs primary key? 0) false 1) true: ");
    scanf("%d", &newAttribute.isPrimary);
    fflush(stdin);

    printf("\nAttribute type: 1) int 2) long 3) float 4) char 5) bool: ");
    scanf("%ld", &newAttribute.type);
    fflush(stdin);

    attributeSize(newAttribute);
    newAttribute.nextAttribute = EMPTY_POINTER;

    long attributeDirection = appendAttribute(dataDictionary, newAttribute);
    reorderAttributes(dataDictionary, currentEntity.attributesPointer, newAttribute.name, attributeDirection);
}

//Eliminar atributo
ATTRIBUTE removeAttribute(FILE* dataDictionary, long currentAttributePointer, const char* attributeName) {
    long currentAttributeDirection = -1;

    fseek(dataDictionary, currentAttributePointer, SEEK_SET);
    fread(&currentAttributeDirection, sizeof(long), 1, dataDictionary);

    if (currentAttributeDirection == -1) {
        ATTRIBUTE emptyAttribute = {0};
        return emptyAttribute;
    } else {
        ATTRIBUTE resultAttribute;
        long nextAttributeDirection;
        long nextHeaderPointer;

        fseek(dataDictionary, currentAttributeDirection, SEEK_SET);
        fread(resultAttribute.name, sizeof(char), DATA_BLOCK_SIZE, dataDictionary);
        nextHeaderPointer = ftell(dataDictionary) + sizeof(bool) + (sizeof(long) * 2);

        if (strcmp(resultAttribute.name, attributeName) == 0) {
            fread(&resultAttribute.isPrimary, sizeof(bool), 1, dataDictionary);
            fread(&resultAttribute.type, sizeof(long), 1, dataDictionary);
            fread(&resultAttribute.size, sizeof(long), 1, dataDictionary);
            fread(&resultAttribute.nextAttribute, sizeof(long), 1, dataDictionary);

            fseek(dataDictionary, currentAttributePointer, SEEK_SET);
            fwrite(&resultAttribute.nextAttribute, sizeof(long), 1, dataDictionary);

            return resultAttribute;
        } else {
            return removeAttribute(dataDictionary, nextHeaderPointer, attributeName);
        }
    }
}

// Captura los atributos de una entidad dada
void captureAttributes(FILE* dataDictionary, ENTITY currentEntity) {
    int keepCapturing = 1;

    printf("\n--- Capturing Attribute for Entity: %s ---\n", currentEntity.name);
    while (keepCapturing) {
        createAttribute(dataDictionary, currentEntity);

        printf("\nDo you want to add another attribute? 1) Yes 0) No: ");
        scanf("%d", &keepCapturing);
        fflush(stdin);
    }
}

// Función trampolín para buscar la entidad y agregar los atributos
void captureAttributesForEntity(FILE* dataDictionary) {
    char entityName[DATA_BLOCK_SIZE];
    ENTITY result;

    // Mostrar las entidades existentes
    showEntities(dataDictionary);

    // Pedir el nombre de la entidad a modificar
    printf("\nEnter the name of the entity: ");
    fgets(entityName, sizeof(entityName), stdin);
    entityName[strcspn(entityName, "\n")] = '\0';  // Eliminar salto de línea

    // Buscar la entidad en el diccionario de datos
    searchEntityByName(dataDictionary, entityName, &result);

    // Si la entidad se encuentra, capturar los atributos
    if (strcmp(result.name, entityName) == 0) {
        captureAttributes(dataDictionary, result);
    } else {
        printf("\nCannot add attributes. Entity not found.\n");
    }
}
