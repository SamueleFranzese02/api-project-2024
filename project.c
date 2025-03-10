#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#define COMMAND 17
#define STRING_DIM 21
#define BUFFER_DIM 100000
#define HASH_TABLE_DIM 10000
#define MIN_HEAP_DIM 5000
#define ORDERS_DIM 30000
#define ORDERS_DIM_WAITING 100000

typedef struct {
    char ingredient[STRING_DIM];
    int weight;
} recipe_ingredient;

typedef struct {
    char key[STRING_DIM];
    recipe_ingredient *recipe_ingredients;
    int recipes_num_ingredient;
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
    ingredient_stock ingredient_stocks[MIN_HEAP_DIM];
    int size;
} min_heap_struct;

typedef struct {
    char key[STRING_DIM];
    min_heap_struct min_heap_ingredient_stocks;
    int weight_tot;
} hash_table_item;

typedef struct {
    hash_table_item *items;
    int size;
    int count;
} hash_table;

typedef struct {
    char order_name[STRING_DIM];
    int quantity;
    int weight;
    int timestamp;
} order_item;

typedef struct {
    order_item *order_items;
    int size;
} order_struct;

// Recipe book methods
int hash_function_recipes(hash_table_recipes *table, char key[]);
hash_table_recipes* hash_table_recipes_create();
hash_table_recipes_item* hash_table_search_recipes(hash_table_recipes *table, char key[]);
void hash_table_insert_recipes(hash_table_recipes *table, char recipe[]);
void hash_table_remove_recipe(hash_table_recipes *table, char recipe[]);
float hash_table_load_factor(hash_table_recipes *table);
float hash_table_load_factor_inventory(hash_table *table);

//Inventory methods
int hash_function(hash_table *table, char key[]);
hash_table* hash_table_create();
hash_table_item* hash_table_search(hash_table *table, char key[]);
void hash_table_insert(hash_table *table, char ingredient[]);

//Min heap methods
void swap(ingredient_stock *x, ingredient_stock *y);
void min_heapify(min_heap_struct *min_heap, int i);
void insert_min_heap(min_heap_struct *min_heap, int expire, int weight);
ingredient_stock extract_min(min_heap_struct *min_heap);
ingredient_stock get_min(min_heap_struct *min_heap);

void remove_expired(hash_table_item *ingredient, int timestamp);
void free_structs(hash_table *inventory, hash_table_recipes* recipe_book);

int main(int arc, char const *argv[]) {
    char buffer[BUFFER_DIM], command[COMMAND], recipe[STRING_DIM], ingredient[STRING_DIM], *input;
    int camion_frequency, camion_weight, quantity, timestamp = 0, result;
    hash_table_recipes *recipe_book;
    hash_table *inventory;
    order_struct *orders_completed, *orders_pending, *orders_sent;

    recipe_book = hash_table_recipes_create();
    inventory = hash_table_create();
    orders_completed = (order_struct *) malloc(sizeof(order_struct));
    orders_completed -> order_items = calloc(ORDERS_DIM, sizeof(order_item));
    orders_completed -> size = 0;
    orders_pending = (order_struct *) malloc(sizeof(order_struct));
    orders_pending -> order_items = calloc(ORDERS_DIM_WAITING, sizeof(order_item));
    orders_pending -> size = 0;
    orders_sent = (order_struct *) malloc(sizeof(order_struct));
    orders_sent -> order_items = calloc(ORDERS_DIM, sizeof(order_item));
    orders_sent -> size = 0;

    if (fgets(buffer, BUFFER_DIM, stdin) != NULL) {
        camion_frequency = atoi(strtok(buffer, " "));
        camion_weight = atoi(strtok(NULL, " "));
        
        while (fgets(buffer, BUFFER_DIM, stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = 0;
            command = strtok(buffer, " ");
            input = strtok(buffer, " ");
            strcpy(command, input);
            if (!strcmp(command, "aggiungi_ricetta")) {
                input = strtok(NULL, " ");
                strcpy(recipe, input);               
                hash_table_insert_recipes(recipe_book, recipe);
            } else if (!strcmp(command, "rimuovi_ricetta")) {
                input = strtok(NULL, " ");
                strcpy(recipe, input);
                hash_table_remove_recipe(recipe_book, recipe);
            } else if (!strcmp(command, "rifornimento")) {
                input = strtok(NULL, " ");
                strcpy(ingredient, input);
                hash_table_insert(inventory, ingredient);
                printf("rifornito\n");
            } else if (!strcmp(command, "ordine")) {
                input = strtok(NULL, " ");
                strcpy(recipe, input);
                quantity = atoi(strtok(NULL, " "));
            } else {
                printf("Not valid\n");
            } 

            timestamp++;
        }   
    }

    free_structs(inventory, recipe_book);
    free(orders_completed ->order_items);
    free(orders_completed);
    free(orders_pending ->order_items);
    free(orders_pending);
    free(orders_sent->order_items);
    free(orders_sent);
    return 0;
}

int hash_function_recipes(hash_table_recipes *table, char key[]) {
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

hash_table_recipes_item* hash_table_search_recipes(hash_table_recipes *table, char key[]) {
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

void hash_table_insert_recipes(hash_table_recipes *table, char recipe[]) {
    int hash, weight, i = 0;
    hash_table_recipes_item *recipe_item;
    char *ingredient;

    hash = hash_function_recipes(table, recipe);
    recipe_item = hash_table_search_recipes(table, recipe);

    if (recipe_item != NULL) {
        printf("ignorato\n");
        return;
    }
    
    while (table -> recipes_items[hash].key[0] != '\0') {
        hash++;
    }    

    table -> recipes_items[hash].key = strdup(recipe);
    table -> recipes_items[hash].recipe_ingredients = calloc(MIN_HEAP_DIM, sizeof(recipe_ingredient));
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

void hash_table_remove_recipe(hash_table_recipes *table, char recipe[]) {
    hash_table_recipes_item *recipe_item;

    recipe_item = hash_table_search_recipes(table, recipe);

    if (recipe_item == NULL) {
        printf("non presente\n");
        return;
    } else if (recipe_item -> recipes_count == 0) {
        free(recipe_item -> recipe_ingredients);
        recipe_item -> key[0] = '\0';
        table -> count--;
        printf("rimossa\n");
        return;
    } else {
        printf("ordini in sospeso\n");
        return;
    }
}

int hash_function(hash_table *table, char key[]) {
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

hash_table_item* hash_table_search(hash_table *table, char key[]) {
    int hash = hash_function(table, key);

    while (table -> items[hash].key[0] != '\0') {
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

void hash_table_insert(hash_table *table, char ingredient[]) {
    int weight, expire, hash;
    hash_table_item *product;

    while (ingredient[0] != '\0') {
        weight = atoi(strtok(NULL, " "));
        expire = atoi(strtok(NULL, " "));
        
        hash = hash_function(table, ingredient);
        product = hash_table_search(table, ingredient);

        if (product != NULL) {
            insert_min_heap(&product -> min_heap_ingredient_stocks, expire, weight);
            product -> weight_tot = product -> weight_tot + weight;
        } else {
            while (table -> items[hash].key[0] != '\0') {
                hash++;
            }

            strcpy(table -> items[hash].key, ingredient);
            table -> items[hash].min_heap_ingredient_stocks.size = 0;
            insert_min_heap(&table -> items[hash].min_heap_ingredient_stocks, expire, weight);
            table -> items[hash].weight_tot = weight;
            table -> count++;
        }

        ingredient = strtok(NULL, " ");
void remove_expired(hash_table_item *ingredient, int timestamp) {
    while (ingredient -> min_heap_ingredient_stocks.size > 0 && ingredient -> min_heap_ingredient_stocks.ingredient_stocks[0].expire <= timestamp) {

        ingredient -> weight_tot = ingredient -> weight_tot - extract_min(&ingredient -> min_heap_ingredient_stocks).weight;

    }
}

float hash_table_load_factor(hash_table_recipes *table) {
    if (table->size == 0) {
        return 0.0;
    }
    return (float)table->count / table->size;
}

float hash_table_load_factor_inventory(hash_table *table) {
    if (table->size == 0) {
        return 0.0;
    }
    return (float)table->count / table->size;
}

void free_structs(hash_table *inventory, hash_table_recipes* recipe_book) {
    free(inventory->items);
    free(inventory);
    
    for (int i = 0; i < recipe_book -> size; i++) {
        if (recipe_book ->recipes_items[i].key[0] != '\0') {
            free(recipe_book ->recipes_items[i].recipe_ingredients);
        }
        
    }
    free(recipe_book->recipes_items);
    free(recipe_book);
    return;
}