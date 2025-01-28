#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define COMMAND 16
#define BUFFER_DIM 100000
#define HASH_TABLE_DIM 40000

typedef struct recipe_ingredient {
    char *ingredient;
    int weight;
} recipe_ingredient;

typedef struct hash_table_recipes_item {
    char *key;
    recipe_ingredient recipe_ingredients[1000];
    int recipes_count;
} hash_table_recipes_item;

typedef struct hash_table_recipes {
    hash_table_recipes_item *recipes_items;
    int size;
    int count;
} hash_table_recipes;


typedef struct ingredient_stock {
    int expire;
    int weight;
} ingredient_stock;

typedef struct hash_table_item {
    char *key;
    ingredient_stock ingredient_stocks[1000];
    int weight_tot;
} hash_table_item;

typedef struct hash_table {
    hash_table_item *items;
    int size;
    int count;
} hash_table;

// Recipe book methods
int hash_function_recipes(hash_table_recipes *table, char *key);
hash_table_recipes* hash_table_recipes_create();
hash_table_recipes_item* hash_table_search_recipes(hash_table_recipes *table, char *key);
void hash_table_insert_recipes(hash_table_recipes *table, char *recipe);

//Inventory methods
int hash_function(hash_table *table, char *key);
hash_table* hash_table_create();
hash_table_item* hash_table_search(hash_table *table, char *key);
void hash_table_insert(hash_table *table, char *ingredient);

int main(int arc, char const *argv[]) {
    char buffer[BUFFER_DIM], *command, *recipe, *ingredient;
    int camion_frequency, camion_weight, weight, quantity, expire_time;
    hash_table_recipes *recipe_book;

    recipe_book = hash_table_recipes_create();

    if (fgets(buffer, BUFFER_DIM, stdin) != NULL) {
        camion_frequency = atoi(strtok(buffer, " "));
        camion_weight = atoi(strtok(NULL, " "));

        printf("%d %d\n", camion_frequency, camion_weight);

        while (fgets(buffer, BUFFER_DIM, stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = 0;
            command = strtok(buffer, " ");
            if (!strcmp(command, "aggiungi_ricetta")) {
                recipe = strtok(NULL, " ");
                hash_table_insert_recipes(recipe_book, recipe);
            } else if (!strcmp(command, "rimuovi_ricetta")) {
                recipe = strtok(NULL, " ");
                printf("%s %s\n", command, recipe);
            } else if (!strcmp(command, "rifornimento")) {
                ingredient = strtok(NULL, " ");
                weight = atoi(strtok(NULL, " "));
                expire_time = atoi(strtok(NULL, " "));

                printf("%s %s %d %d", command, ingredient, weight, expire_time);
                while ((ingredient = strtok(NULL, " ")) != NULL) {
                    weight = atoi(strtok(NULL, " "));
                    expire_time = atoi(strtok(NULL, " "));
                    printf(" %s %d %d", ingredient, weight, expire_time);
                }

                printf("\n");
            } else if (!strcmp(command, "ordine")) {
                recipe = strtok(NULL, " ");
                quantity = atoi(strtok(NULL, " "));
                printf("%s %s %d\n", command, recipe, quantity);
            } else {
                printf("Not valid\n");
            } 
        }   
    }

    return 0;
}

int hash_function_recipes(hash_table_recipes *table, char *key) {
    unsigned long hash = 5381;
    int character;

    while ((character = *key++)) {
        hash = ((hash << 5) + hash) + character;
    }

    return hash % table -> size;
}

hash_table_recipes* hash_table_recipes_create() {
    hash_table_recipes *table = (hash_table_recipes *) malloc(sizeof(hash_table_recipes));
    table -> size = HASH_TABLE_DIM;
    table -> count = 0;

    table -> recipes_items = calloc(table -> size, sizeof(hash_table_recipes_item));

    return table;
}

hash_table_recipes_item* hash_table_search_recipes(hash_table_recipes *table, char *key) {
    int hash = hash_function_recipes(table, key);

    while (table -> recipes_items[hash].key != NULL) {
        if (strcmp(key, table -> recipes_items[hash].key) == 0) {
            return &table -> recipes_items[hash];
        } else {
            hash++;
            if (hash >= table -> size) {
                return NULL;
            }
        }
    }

    return NULL;
}

void hash_table_insert_recipes(hash_table_recipes *table, char *recipe) {
    int hash, weight, i = 0;
    hash_table_recipes_item *recipe_item;
    char *ingredient;

    hash = hash_function_recipes(table, recipe);
    recipe_item = hash_table_search_recipes(table, recipe);

    if (recipe_item != NULL) {
        printf("ignorato\n");
        return;
    }
    
    while (table -> recipes_items[hash].key != NULL) {
        hash++;
    }

    table -> recipes_items[hash].key = strdup(recipe);
    table -> recipes_items[hash].recipes_count = 0;
    table -> count++;

    ingredient = strtok(NULL, " ");
    while (ingredient != NULL) {
        weight = atoi(strtok(NULL, " "));

        table -> recipes_items[hash].recipe_ingredients[i].ingredient = strdup(ingredient);
        table -> recipes_items[hash].recipe_ingredients[i].weight = weight;

        i++;
        ingredient = strtok(NULL, " ");
    }

    printf("aggiunta\n");
    return;
}

int hash_function(hash_table *table, char *key) {
    unsigned long hash = 5381;
    int character;

    while ((character = *key++)) {
        hash = ((hash << 5) + hash) + character;
    }

    return hash % table -> size;
}

hash_table* hash_table_create() {
    hash_table *table = (hash_table *) malloc(sizeof(hash_table));
    table -> size = HASH_TABLE_DIM;
    table -> count = 0;

    table -> items = calloc(table -> size, sizeof(hash_table_item));

    return table;
}

hash_table_item* hash_table_search(hash_table *table, char *key) {
    int hash = hash_function(table, key);

    while (table -> items[hash].key != NULL) {
        if (strcmp(key, table -> items[hash].key) == 0) {
            return &table -> items[hash];
        } else {
            hash++;
            if (hash >= table -> size) {
                return NULL;
            }
        }
    }

    return NULL;
}