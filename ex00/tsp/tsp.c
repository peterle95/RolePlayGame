#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#define MAX_CITIES 11

typedef struct {
    float x;
    float y;
} City;

// compute the distance between two points
float distance(float x0, float y0, float x1, float y1) 
{
    float diff[2] = {
        x1 - x0, 
        y1 - y0
    };

    return sqrtf(diff[0] * diff[0] + diff[1] * diff[1]);
}

// Calculate total path distance for given order of cities
float calculate_path_length(City *cities, int *path, int n) {
    float total = 0;
    for (int i = 0; i < n - 1; i++) {
        total += distance(
            cities[path[i]].x, cities[path[i]].y,
            cities[path[i + 1]].x, cities[path[i + 1]].y
        );
    }
    // Add distance back to start
    total += distance(
        cities[path[n - 1]].x, cities[path[n - 1]].y,
        cities[path[0]].x, cities[path[0]].y
    );
    return total;
}

// Recursive function to try all possible paths
void find_shortest_path(City *cities, int n, int *curr_path, bool *visited, 
                       int pos, float *shortest, int *best_path) {
    if (pos == n) {
        float length = calculate_path_length(cities, curr_path, n);
        if (length < *shortest) {
            *shortest = length;
            memcpy(best_path, curr_path, n * sizeof(int));
        }
        return;
    }

    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            visited[i] = true;
            curr_path[pos] = i;
            find_shortest_path(cities, n, curr_path, visited, pos + 1, shortest, best_path);
            visited[i] = false;
        }
    }
}

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    City cities[MAX_CITIES];
    int n = 0;
    float x, y;

    // Read cities
    while (n < MAX_CITIES && scanf("%f, %f", &x, &y) == 2) {
        cities[n].x = x;
        cities[n].y = y;
        n++;
    }

    if (n == 0) return 1;

    // Initialize arrays for path finding
    int curr_path[MAX_CITIES];
    int best_path[MAX_CITIES];
    bool visited[MAX_CITIES] = {false};
    float shortest = INFINITY;

    // Start with city 0 and try all possible paths
    visited[0] = true;
    curr_path[0] = 0;
    find_shortest_path(cities, n, curr_path, visited, 1, &shortest, best_path);

    // Print result
    printf("%.2f\n", shortest);
    return 0;
}