#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define COMMAND 16
#define BUFFER_DIM 100000

int main(int arc, char const *argv[]) {
    char buffer[BUFFER_DIM], *command, *recipe, *ingredient;
    int camion_frequency, camion_weight, weight, quantity, expire_time;

    if (fgets(buffer, BUFFER_DIM, stdin) != NULL) {
        camion_frequency = atoi(strtok(buffer, " "));
        camion_weight = atoi(strtok(NULL, " "));

        printf("%d %d\n", camion_frequency, camion_weight);

        while (fgets(buffer, BUFFER_DIM, stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = 0;
            command = strtok(buffer, " ");
            if (!strcmp(command, "aggiungi_ricetta")) {
                recipe = strtok(NULL, " ");
                ingredient = strtok(NULL, " ");
                weight = atoi(strtok(NULL, " "));
                printf("%s %s %s %d", command, recipe, ingredient, weight);

                while ((ingredient = strtok(NULL, " ")) != NULL) {
                    weight = atoi(strtok(NULL, " "));
                    printf(" %s %d", ingredient, weight);
                }
                printf("\n");
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