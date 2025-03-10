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

int make_order(hash_table *inventory, hash_table_recipes *recipe_book, order_struct *orders_completed, order_struct *orders_pending, char recipe[], int quantity, int timestamp, int waiting, int current_timestamp);
void remove_expired(hash_table_item *ingredient, int timestamp);
void send_orders_completed(hash_table_recipes *table, order_struct *orders_completed, order_struct *orders_sent, int camion_weight);
void merge(order_struct *orders_sent, int p, int q, int r);

int binary_search(order_struct *orders_completed, int timestamp);
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
            if (timestamp % camion_frequency == 0 && timestamp != 0) {
                send_orders_completed(recipe_book, orders_completed, orders_sent, camion_weight);
            }
            
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
                
                if (orders_pending -> size != 0) {
                    for (int i = 0; i < orders_pending -> size;) {
                        result = make_order(inventory, recipe_book, orders_completed, orders_pending, orders_pending -> order_items[i].order_name, orders_pending -> order_items[i].quantity, orders_pending -> order_items[i].timestamp, 1, timestamp);
                        
                        if (result == 2) {
                            memmove(orders_pending -> order_items + i, orders_pending -> order_items + i + 1, (orders_pending -> size - i - 1) * sizeof(order_item));
                            orders_pending -> size--;
                        } else {
                            i++;
                        }
                        
                    }
                }
                
            } else if (!strcmp(command, "ordine")) {
                input = strtok(NULL, " ");
                strcpy(recipe, input);
                quantity = atoi(strtok(NULL, " "));
                result = make_order(inventory, recipe_book, orders_completed, orders_pending, recipe, quantity, timestamp, 0, timestamp);

                if (result == 1 || result == -1) {
                    printf("accettato\n");
                } else if (result == -2) {
                    printf("rifiutato\n");

                }
                                
            } else {
                printf("Not valid\n");
            } 

            timestamp++;
        } 

        if (timestamp % camion_frequency == 0 && timestamp != 0) {
            send_orders_completed(recipe_book, orders_completed, orders_sent, camion_weight);
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
int binary_search(order_struct *orders_completed, int timestamp) {
    int right, left, mid;

    left = 0;
    right = orders_completed -> size;

    while(left < right) {
        mid = left + (right - left) / 2;

        if (orders_completed -> order_items[mid].timestamp < timestamp) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }

    return left;
}

void remove_expired(hash_table_item *ingredient, int timestamp) {
    while (ingredient -> min_heap_ingredient_stocks.size > 0 && ingredient -> min_heap_ingredient_stocks.ingredient_stocks[0].expire <= timestamp) {

        ingredient -> weight_tot = ingredient -> weight_tot - extract_min(&ingredient -> min_heap_ingredient_stocks).weight;

    }
}

int make_order(hash_table *inventory, hash_table_recipes *recipe_book, order_struct *orders_completed, order_struct *orders_pending, char recipe[], int quantity, int timestamp, int waiting, int current_timestamp) {
    hash_table_recipes_item *recipe_item;
    hash_table_item **ingredient_item = calloc(100, sizeof(hash_table_item *));
    recipe_ingredient *recipe_ingredients;
    int i = 0, j = 0, weight_needed, weight_current, order_weight = 0, timestamp_insert;

    recipe_item = hash_table_search_recipes(recipe_book, recipe);  

    if (recipe_item != NULL) {
        recipe_ingredients = recipe_item -> recipe_ingredients;
        while (recipe_ingredients[i].ingredient[0] != '\0') {
            ingredient_item[j] = hash_table_search(inventory, recipe_ingredients[i].ingredient);
            if (ingredient_item[j] == NULL) {
                if (waiting == 0){
                    strcpy(orders_pending -> order_items[orders_pending -> size].order_name, recipe);
                    orders_pending -> order_items[orders_pending -> size].quantity = quantity;
                    orders_pending -> order_items[orders_pending -> size].timestamp = timestamp;
                    orders_pending -> order_items[orders_pending -> size].weight = 0;
                    orders_pending -> size++;
                    recipe_item -> recipes_count++;
                }
                return -1;
            }
            
            weight_needed = recipe_ingredients[i].weight * quantity;
            remove_expired(ingredient_item[j], current_timestamp);
            if (weight_needed > ingredient_item[j] -> weight_tot) {
                if (waiting == 0) {
                    strcpy(orders_pending -> order_items[orders_pending -> size].order_name, recipe);

                    orders_pending -> order_items[orders_pending -> size].quantity = quantity;
                    orders_pending -> order_items[orders_pending -> size].timestamp = timestamp;
                    orders_pending -> order_items[orders_pending -> size].weight = 0;
                    orders_pending -> size++;
                    recipe_item -> recipes_count++;
                }
                return -1;
            }
            i++;
            j++;
        }

        i = 0, j = 0;
        recipe_ingredients = recipe_item -> recipe_ingredients;

        while (recipe_ingredients[i].ingredient[0] != '\0') {
            weight_needed = recipe_ingredients[i].weight * quantity;
            order_weight = order_weight + weight_needed;

            while (weight_needed != 0) {
                weight_current = ingredient_item[j] -> min_heap_ingredient_stocks.ingredient_stocks[0].weight;
                if (weight_needed - weight_current < 0) {
                    ingredient_item[j] -> min_heap_ingredient_stocks.ingredient_stocks[0].weight = ingredient_item[j] -> min_heap_ingredient_stocks.ingredient_stocks[0].weight - weight_needed;
                    ingredient_item[j] -> weight_tot = ingredient_item[j] -> weight_tot - weight_needed;
                    weight_needed = 0;
                } else if (weight_needed - weight_current > 0) {
                    weight_needed = weight_needed - weight_current;
                    ingredient_item[j] -> weight_tot = ingredient_item[j] -> weight_tot - weight_current;
                    extract_min(&ingredient_item[j] -> min_heap_ingredient_stocks);
                } else {
                    weight_needed = weight_needed - weight_current;
                    ingredient_item[j] -> weight_tot = ingredient_item[j] -> weight_tot - weight_current;
                    extract_min(&ingredient_item[j] -> min_heap_ingredient_stocks);
                }
            } 
            i++;
            j++;
        }
        
        if (waiting == 0) {
            strcpy(orders_completed -> order_items[orders_completed -> size].order_name, recipe);
            orders_completed -> order_items[orders_completed -> size].quantity = quantity;
            orders_completed -> order_items[orders_completed -> size].timestamp = timestamp;
            orders_completed -> order_items[orders_completed -> size].weight = order_weight;
            orders_completed -> size++;
            recipe_item -> recipes_count++;
            
        } else {
            if (orders_completed -> size != 0) {
                timestamp_insert = binary_search(orders_completed, timestamp);
                if (timestamp_insert != orders_completed -> size) {
                    memmove(orders_completed -> order_items + timestamp_insert + 1, orders_completed -> order_items + timestamp_insert, (orders_completed -> size - timestamp_insert) * sizeof(order_item));
                }
            } else {
                timestamp_insert = 0;
            }
            strcpy(orders_completed -> order_items[timestamp_insert].order_name, recipe);
            orders_completed -> order_items[timestamp_insert].quantity = quantity;
            orders_completed -> order_items[timestamp_insert].timestamp = timestamp;
            orders_completed -> order_items[timestamp_insert].weight = order_weight;
            orders_completed -> size++;

            return 2;
        }
        
    } else {
        return -2;
    }

    return 1;
}

void merge(order_struct *orders_sent, int p, int q, int r) {
    int len1 = q - p + 1;
    int len2 = r - q;

    order_item *left = (order_item *) malloc((len1 + 1) * sizeof(order_item));
    order_item *right = (order_item *) malloc((len2 + 1) * sizeof(order_item));

    for (int i = 0; i < len1; i++) {
        left[i] = orders_sent -> order_items[p + i];
    }
    for (int i = 0; i < len2; i++) {
        right[i] = orders_sent -> order_items[q + i + 1];
    }
    
    left[len1].weight = INT_MIN;
    right[len2].weight = INT_MIN;

    int i = 0, j = 0;
    for (int k = p; k <= r; k++) {
        if (left[i].weight > right[j].weight || (left[i].weight == right[j].weight && left[i].timestamp <= right[j].timestamp)) {
            orders_sent -> order_items[k] = left[i];
            i++;
        } else {
            orders_sent -> order_items[k] = right[j];
            j++;
        }
    }
    
    free(left);
    free(right);
}

void merge_sort(order_struct *orders_sent, int p, int r) {
    int q;

    if (p < r - 1) {
        q = (p + r) / 2;

        merge_sort(orders_sent, p, q);
        merge_sort(orders_sent, q + 1, r);
        merge(orders_sent, p, q, r);
    } else {
        if (orders_sent -> order_items[p].weight < orders_sent -> order_items[r].weight) {
            order_item tmp = orders_sent -> order_items[r];
            orders_sent -> order_items[r] = orders_sent -> order_items[p];
            orders_sent -> order_items[p] = tmp;
        }   
    }
}

void send_orders_completed(hash_table_recipes* table, order_struct *orders_completed, order_struct *orders_sent, int camion_weight) {
    int num_items = 0;

    for (int i = 0, j = orders_sent -> size; i < orders_completed -> size; i++, j++) {
        if (orders_completed -> order_items[i].weight <= camion_weight) {
            camion_weight = camion_weight - orders_completed -> order_items[i].weight;
            strcpy(orders_sent -> order_items[j].order_name, orders_completed -> order_items[i].order_name);
            orders_sent -> order_items[j].quantity = orders_completed -> order_items[i].quantity;
            orders_sent -> order_items[j].timestamp = orders_completed -> order_items[i].timestamp;
            orders_sent -> order_items[j].weight = orders_completed -> order_items[i].weight;

            hash_table_search_recipes(table, orders_completed -> order_items[i].order_name) -> recipes_count--;
            num_items++;
        } else {
            break;
        }
    }

    if (num_items > 0) {
        memmove(orders_completed -> order_items, orders_completed -> order_items + num_items, (orders_completed -> size - num_items) * sizeof(order_item));
        orders_completed -> size = orders_completed -> size - num_items;
        orders_sent -> size = orders_sent -> size + num_items;
    } else if (num_items == 0) {
        printf("camioncino vuoto\n");
        return;
    }
    
    
    merge_sort(orders_sent, 0, orders_sent -> size - 1);
    for (int i = 0; i < orders_sent -> size; i++) {
        printf("%d %s %d\n", orders_sent -> order_items[i].timestamp, orders_sent -> order_items[i].order_name, orders_sent -> order_items[i].quantity);
        orders_sent -> order_items[i].order_name[0] = '\0';
        orders_sent -> order_items[i].timestamp = 0;
        orders_sent -> order_items[i].weight = 0;
    }

    orders_sent -> size = 0;
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