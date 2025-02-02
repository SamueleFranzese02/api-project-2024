#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define COMMAND 16
#define BUFFER_DIM 100000
#define HASH_TABLE_DIM 40000

typedef struct {
    char *ingredient;
    int weight;
} recipe_ingredient;

typedef struct {
    char *key;
    recipe_ingredient *recipe_ingredients;
    int recipes_count;
} hash_table_recipes_item;

typedef struct {
    hash_table_recipes_item *recipes_items;
    int size;
    int count;
} hash_table_recipes;


typedef struct {
    int expire;
    int weight;
} ingredient_stock;

typedef struct {
    ingredient_stock ingredient_stocks[1000];
    int size;
} min_heap_struct;

typedef struct {
    char *key;
    min_heap_struct min_heap_ingredient_stocks;
    int weight_tot;
} hash_table_item;

typedef struct {
    hash_table_item *items;
    int size;
    int count;
} hash_table;

// Recipe book methods
int hash_function_recipes(hash_table_recipes *table, char *key);
hash_table_recipes* hash_table_recipes_create();
hash_table_recipes_item* hash_table_search_recipes(hash_table_recipes *table, char *key);
void hash_table_insert_recipes(hash_table_recipes *table, char *recipe);
void hash_table_remove_recipe(hash_table_recipes *table, char *recipe);

//Inventory methods
int hash_function(hash_table *table, char *key);
hash_table* hash_table_create();
hash_table_item* hash_table_search(hash_table *table, char *key);
void hash_table_insert(hash_table *table, char *ingredient);

//Min heap methods
void swap(ingredient_stock *x, ingredient_stock *y);
void min_heapify(min_heap_struct *min_heap, int i);
void insert_min_heap(min_heap_struct *min_heap, int expire, int weight);
ingredient_stock extract_min(min_heap_struct *min_heap);
ingredient_stock get_min(min_heap_struct *min_heap);

int main(int arc, char const *argv[]) {
    char buffer[BUFFER_DIM], *command, *recipe, *ingredient;
    int camion_frequency, camion_weight, quantity;
    hash_table_recipes *recipe_book;
    hash_table *inventory;

    recipe_book = hash_table_recipes_create();
    inventory = hash_table_create();

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
                hash_table_remove_recipe(recipe_book, recipe);
            } else if (!strcmp(command, "rifornimento")) {
                ingredient = strtok(NULL, " ");
                hash_table_insert(inventory, ingredient);
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
    table -> recipes_items[hash].recipe_ingredients = calloc(1000, sizeof(recipe_ingredient));
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

void hash_table_remove_recipe(hash_table_recipes *table, char *recipe) {
    hash_table_recipes_item *recipe_item;

    recipe_item = hash_table_search_recipes(table, recipe);

    if (recipe_item == NULL) {
        printf("non presente\n");
        return;
    } else if (recipe_item -> recipes_count == 0) {
        free(recipe_item -> recipe_ingredients);
        recipe_item -> key = NULL;
        table -> count--;
        printf("rimossa\n");
        return;
    } else {
        printf("ordini in sospeso\n");
        return;
    }
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

void swap(ingredient_stock *x, ingredient_stock *y) {
    ingredient_stock temp = *x;
    *x = *y;
    *y = temp;
}

void min_heapify(min_heap_struct *min_heap, int i) {
    int smallest, l, r;

    l = 2 * i + 1;
    r = 2 * i + 2;

    if (l < min_heap -> size && min_heap -> ingredient_stocks[l].expire < min_heap -> ingredient_stocks[i].expire) {
        smallest = l;
    } else {
        smallest = i;
    }

    if (r < min_heap -> size && min_heap -> ingredient_stocks[r].expire < min_heap -> ingredient_stocks[smallest].expire) {
        smallest = r;
    }

    if (smallest != i) {
        swap(&min_heap -> ingredient_stocks[i], &min_heap -> ingredient_stocks[smallest]);
        min_heapify(min_heap, smallest);
    }
}

void insert_min_heap(min_heap_struct *min_heap, int expire, int weight) {
    int i;

    i = min_heap -> size;
    min_heap -> ingredient_stocks[i].expire = expire;
    min_heap -> ingredient_stocks[i].weight = weight;
    (min_heap -> size)++;

    while (i != 0 && min_heap -> ingredient_stocks[(i - 1) / 2].expire > min_heap -> ingredient_stocks[i].expire) {
        swap(&min_heap -> ingredient_stocks[i], &min_heap -> ingredient_stocks[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

ingredient_stock extract_min(min_heap_struct *min_heap) {
    ingredient_stock root;
    
    if (min_heap -> size == 1) {
        (min_heap -> size)--;
        return min_heap -> ingredient_stocks[0];
    }
    
    root = min_heap -> ingredient_stocks[0];
    min_heap -> ingredient_stocks[0] = min_heap -> ingredient_stocks[(min_heap -> size) - 1];
    (min_heap -> size)--;
    min_heapify(min_heap, 0);

    return root;
}

ingredient_stock get_min(min_heap_struct *min_heap) {
    return min_heap -> ingredient_stocks[0];
}

void hash_table_insert(hash_table *table, char *ingredient) {
    int weight, expire, hash;
    hash_table_item *product;

    while (ingredient != NULL) {
        weight = atoi(strtok(NULL, " "));
        expire = atoi(strtok(NULL, " "));
        
        hash = hash_function(table, ingredient);
        product = hash_table_search(table, ingredient);

        if (product != NULL) {
            insert_min_heap(&product -> min_heap_ingredient_stocks, expire, weight);
            product -> weight_tot = product -> weight_tot + weight;
        } else {
            while (table -> items[hash].key != NULL) {
                hash++;
            }

            table -> items[hash].key = strdup(ingredient);
            table -> items[hash].min_heap_ingredient_stocks.size = 0;
            insert_min_heap(&table -> items[hash].min_heap_ingredient_stocks, expire, weight);
            table -> items[hash].weight_tot = weight;
            table -> count++;
        }

        ingredient = strtok(NULL, " ");
    }
}